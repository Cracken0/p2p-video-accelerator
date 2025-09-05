<template>
  <div class="container">
    <div class="card">
      <div class="title">{{ video?.title || '正在加载...' }}</div>
      <video ref="player" controls style="width:100%; border-radius:8px; background:#000; height: 420px"></video>
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
    return { video: null, stream: null, stats: {}, timer: null, playing: false, currentTime: 0, duration: 0 }
  },
  async mounted(){
    const rid = this.$route.query.rid
    this.video = await getVideo(rid)
    this.stream = await getStream(rid)
    const el = this.$refs.player
    el.src = this.stream.url
    el.addEventListener('loadedmetadata', () => { this.duration = el.duration || this.video.duration })
    el.addEventListener('timeupdate', () => { this.currentTime = el.currentTime })
    this.timer = setInterval(async () => { this.stats = await getStats() }, 500)
  },
  unmounted(){ if (this.timer) clearInterval(this.timer) },
  methods: {
    togglePlay(){ const el = this.$refs.player; if (el.paused) { el.play(); this.playing = true } else { el.pause(); this.playing = false } },
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
