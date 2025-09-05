<template>
  <div class="container">
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
  async mounted(){ this.history = await listHistory() },
  methods: {
    formatTime(ts){ const d = new Date(ts); const pad = n => String(n).padStart(2, '0'); return `${d.getFullYear()}/${pad(d.getMonth()+1)}/${pad(d.getDate())} ${pad(d.getHours())}:${pad(d.getMinutes())}` },
    open(h){ this.$router.push({ path: '/video', query: { rid: h.rid } }) }
  }
}
</script>

<style scoped>
.row { display: flex; justify-content: space-between; }
.left { display: flex; flex-direction: column; gap: 6px; }
.title { font-weight: bold; font-size: 16px; }
.kv { color: #555; }
.kv span + span { margin-left: 16px; }
</style>
