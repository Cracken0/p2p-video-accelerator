<template>
  <div class="container">
    <div class="card">
      <div class="title">{{ video?.title || '正在加载...' }}</div>
      <video ref="player" controls preload="auto" style="width:100%; border-radius:8px; background:#000; height: 420px"></video>
      <div class="controls">
        <input type="range" min="0" :max="duration" step="0.1" v-model.number="currentTime" @input="seek" @mousedown="onSeekStart" @mouseup="onSeekEnd" @change="onSeekEnd" />
        <div class="bottom">
          <button class="btn" @click="togglePlay">{{ playing ? '暂停' : '播放' }}</button>
          <StatusBar :stats="computedStats" />
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import { getVideo, getStream } from '@/services/videoApi'
import { getStats, listPeers } from '@/services/peerApi'
import { addHistory, updateHistory, listHistory } from '@/services/historyApi'
import StatusBar from '@/components/StatusBar.vue'

export default {
  name: 'VideoPage',
  components: { StatusBar },
  data(){
    return { video: null, stream: null, stats: {}, peers: [], timer: null, playing: false, currentTime: 0, duration: 0, isPending: false, ready: false, loadError: '', isSeeking: false, historyId: null, historyTimer: null }
  },
  async mounted(){
    const rid = this.$route.query.rid
    this.video = await getVideo(rid)
    this.stream = await getStream(rid)
    const el = this.$refs.player
    el.src = this.getVideoUrl(this.stream.url)
    // 触发加载，监听可播放与错误
    el.load()
    el.addEventListener('canplay', () => { this.ready = true })
    el.addEventListener('error', () => { this.loadError = (el.error && el.error.message) || '加载视频失败'; this.ready = false })
    el.addEventListener('loadedmetadata', () => { this.duration = el.duration || this.video.duration })
    el.addEventListener('timeupdate', () => { 
      if (!this.isSeeking) {
        this.currentTime = el.currentTime 
      }
    })
    el.addEventListener('play', () => { this.playing = true })
    el.addEventListener('pause', () => { this.playing = false })
    this.timer = setInterval(async () => { 
      this.stats = await getStats()
      this.peers = await listPeers()
    }, 1000)
    
    // 创建历史记录
    await this.createHistoryRecord()
  },
  computed: {
    computedStats() {
      // 计算在线且启用的节点数
      const onlineEnabledPeers = this.peers.filter(peer => peer.is_online && peer.is_enabled).length
      
      // 获取节点一的数据（第一个节点）
      const peer1 = this.peers.length > 0 ? this.peers[0] : null
      
      return {
        ...this.stats,
        total_peers: onlineEnabledPeers,
        // 只显示节点一的下行和上行速率
        total_download_speed: peer1 ? peer1.download_speed : 0,
        total_upload_speed: peer1 ? peer1.upload_speed : 0
      }
    }
  },
  unmounted(){ 
    if (this.timer) clearInterval(this.timer)
    if (this.historyTimer) clearInterval(this.historyTimer)
  },
  methods: {
    getVideoUrl(url) {
      // 自动识别URL类型并返回合适的格式
      if (!url || typeof url !== 'string') {
        return url
      }
      
      // 检查是否为HTTP/HTTPS URL
      if (url.startsWith('http://') || url.startsWith('https://')) {
        return url
      }
      
      // 检查是否为file:// URL
      if (url.startsWith('file://')) {
        return url
      }
      
      // 检查是否为本地文件路径（以 / 开头）
      if (url.startsWith('/')) {
        return `file://${url}`
      }
      
      // 其他情况（如相对路径），直接返回
      return url
    },
    async togglePlay(){
      const el = this.$refs.player
      if (this.isPending) return
      if (el.paused) {
        this.isPending = true
        try {
          if (!this.ready) {
            await new Promise((resolve) => {
              const handler = () => { el.removeEventListener('canplay', handler); resolve() }
              el.addEventListener('canplay', handler, { once: true })
              // 确保触发加载流程
              try { el.load() } catch (_) {}
            })
          }
          await el.play()
        } catch (e) {
          // 忽略因快速切换或策略限制导致的 play() 中断/拒绝
        } finally {
          this.isPending = false
        }
      } else {
        el.pause()
      }
    },
    seek(){ 
      const el = this.$refs.player
      if (el && this.ready) {
        // 立即更新视频位置
        el.currentTime = this.currentTime
      }
    },
    onSeekStart() {
      this.isSeeking = true
    },
    onSeekEnd() {
      if (this.isSeeking) {
        this.isSeeking = false
        // 确保最终位置同步
        const el = this.$refs.player
        if (el && this.ready) {
          el.currentTime = this.currentTime
          // 短暂延迟后重新启用timeupdate更新
          setTimeout(() => {
            if (!this.isSeeking) {
              this.currentTime = el.currentTime
            }
          }, 100)
        }
      }
    },
    async createHistoryRecord() {
      try {
        const rid = this.$route.query.rid
        console.log('处理历史记录，rid:', rid)
        
        const currentStats = await getStats()
        console.log('当前统计信息:', currentStats)
        
        // 获取所有历史记录，检查是否已存在相同rid的记录
        const allHistory = await listHistory()
        const existingRecord = allHistory.find(record => record.rid === rid)
        
        const historyData = {
          rid: rid,
          title: this.video?.title || '',
          file_size: this.video?.file_size || 0,
          playback_progress: 0.0,
          cache_exists: true,
          avg_download_speed: currentStats.total_download_speed || 0
        }
        
        if (existingRecord) {
          // 如果存在相同rid的记录，更新它
          console.log('找到已存在的记录，更新:', existingRecord.id)
          const result = await updateHistory(existingRecord.id, historyData)
          console.log('历史记录更新成功:', result)
          this.historyId = existingRecord.id
        } else {
          // 如果不存在，创建新记录
          console.log('创建新的历史记录')
          const result = await addHistory(historyData)
          console.log('历史记录创建成功:', result)
          this.historyId = result.id
        }
        
        // 启动播放进度更新定时器
        this.startHistoryProgressUpdate()
        
      } catch (error) {
        console.error('处理历史记录失败:', error)
      }
    },
    startHistoryProgressUpdate() {
      // 每秒更新播放进度
      this.historyTimer = setInterval(async () => {
        if (this.historyId && this.duration > 0) {
          const progress = (this.currentTime / this.duration) * 100
          try {
            const rid = this.$route.query.rid
            const currentStats = await getStats()
            
            const updateData = {
              rid: rid,
              title: this.video?.title || '',
              file_size: this.video?.file_size || 0,
              playback_progress: parseFloat(progress.toFixed(2)), // 保留两位小数
              cache_exists: true,
              avg_download_speed: currentStats.total_download_speed || 0,
              play_time: new Date().toISOString() // 更新为当前时间
            }
            
            await updateHistory(this.historyId, updateData)
            console.log(`更新播放进度: ${progress.toFixed(2)}%`)
          } catch (error) {
            console.error('更新播放进度失败:', error)
          }
        }
      }, 1000)
    }
  }
}
</script>

<style scoped>
.title { font-weight: bold; font-size: 18px; margin-bottom: 12px; }
.controls { margin-top: 12px; }
.controls .bottom { display: flex; justify-content: space-between; align-items: center; margin-top: 8px; }
input[type=range]{ width: 100%; }
</style>
