<template>
  <div class="container">
    <div class="header">
      <h2>历史记录</h2>
      <button @click="refresh" class="refresh-btn">刷新</button>
    </div>
    <div class="card" v-for="h in history" :key="h.id" @click="open(h)" style="cursor:pointer;">
      <div class="row">
        <div class="left">
          <div class="title">{{ h.title }}</div>
          <div class="kv">
            <span>时间：{{ formatTime(h.play_time) }}</span>
            <span>  大小(MB):{{ h.file_size }}</span>
          </div>
          <div class="kv">
            <span>播放进度%：{{ h.playback_progress }}</span>
            <span>  缓存存在：{{ h.cache_exists ? '是' : '否' }}</span>
          </div>
          <div class="kv">
            <span>平均下行(MB/s): {{ h.avg_download_speed }}</span>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import { listHistory } from '@/services/historyApi'
export default {
  name: 'HistoryPage',
  data(){ return { history: [] } },
  async mounted(){ 
    const historyData = await listHistory()
    // 按照play_time降序排序，最晚发生的放在最上面
    this.history = historyData.sort((a, b) => {
      const timeA = new Date(a.play_time).getTime()
      const timeB = new Date(b.play_time).getTime()
      return timeB - timeA // 降序排序
    })
  },
  methods: {
    formatTime(ts){ const d = new Date(ts); const pad = n => String(n).padStart(2, '0'); return `${d.getFullYear()}/${pad(d.getMonth()+1)}/${pad(d.getDate())} ${pad(d.getHours())}:${pad(d.getMinutes())}` },
    open(h){ this.$router.push({ path: '/video', query: { rid: h.rid } }) },
    async refresh() {
      const historyData = await listHistory()
      // 按照play_time降序排序，最晚发生的放在最上面
      this.history = historyData.sort((a, b) => {
        const timeA = new Date(a.play_time).getTime()
        const timeB = new Date(b.play_time).getTime()
        return timeB - timeA // 降序排序
      })
    }
  }
}
</script>

<style scoped>
.header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 20px;
  padding-bottom: 10px;
  border-bottom: 1px solid #eee;
}

.header h2 {
  margin: 0;
  color: #333;
}

.refresh-btn {
  background: #007bff;
  color: white;
  border: none;
  padding: 8px 16px;
  border-radius: 4px;
  cursor: pointer;
  font-size: 14px;
}

.refresh-btn:hover {
  background: #0056b3;
}

.row { display: flex; justify-content: space-between; }
.left { display: flex; flex-direction: column; gap: 6px; }
.title { font-weight: bold; font-size: 16px; }
.kv { color: #555; }
.kv span + span { margin-left: 16px; }
</style>
