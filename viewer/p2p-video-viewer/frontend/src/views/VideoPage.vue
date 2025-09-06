<template>
  <div class="container">
    <div class="card">
      <div class="title">{{ video?.title || '正在加载...' }}</div>
      <video ref="player" controls preload="auto" style="width:100%; border-radius:8px; background:#000; height: 420px"></video>
      <div class="controls">
        <input type="range" min="0" :max="duration" step="0.1" v-model.number="currentTime" @input="seek" />
        <div class="bottom">
          <button class="btn" @click="togglePlay">{{ playing ? '暂停' : '播放' }}</button>
          <StatusBar :stats="stats" />
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import { getVideo, getStream } from '@/services/videoApi'
import { getStats } from '@/services/peerApi'
import StatusBar from '@/components/StatusBar.vue'

export default {
  name: 'VideoPage',
  components: { StatusBar },
  data(){
    return { video: null, stream: null, stats: {}, timer: null, playing: false, currentTime: 0, duration: 0, isPending: false, ready: false, loadError: '' }
  },
  async mounted(){
    const rid = this.$route.query.rid
    this.video = await getVideo(rid)
    this.stream = await getStream(rid)
    const el = this.$refs.player
    el.src = this.stream.url
    // 触发加载，监听可播放与错误
    el.load()
    el.addEventListener('canplay', () => { this.ready = true })
    el.addEventListener('error', () => { this.loadError = (el.error && el.error.message) || '加载视频失败'; this.ready = false })
    el.addEventListener('loadedmetadata', () => { this.duration = el.duration || this.video.duration })
    el.addEventListener('timeupdate', () => { this.currentTime = el.currentTime })
    el.addEventListener('play', () => { this.playing = true })
    el.addEventListener('pause', () => { this.playing = false })
    this.timer = setInterval(async () => { this.stats = await getStats() }, 500)
  },
  unmounted(){ if (this.timer) clearInterval(this.timer) },
  methods: {
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
    seek(){ const el = this.$refs.player; el.currentTime = this.currentTime }
  }
}
</script>

<style scoped>
.title { font-weight: bold; font-size: 18px; margin-bottom: 12px; }
.controls { margin-top: 12px; }
.controls .bottom { display: flex; justify-content: space-between; align-items: center; margin-top: 8px; }
input[type=range]{ width: 100%; }
</style>
