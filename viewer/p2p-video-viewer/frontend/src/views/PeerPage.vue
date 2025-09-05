<template>
  <div class="container">
    <div class="card">
      <div class="top">
        <span>自身NAT穿透类型：{{ stats.nat_type || '未知' }}</span>
        <span>  总节点数：{{ stats.total_peers || 0 }}</span>
      </div>
      <div class="table">
        <div class="thead">
          <div>pid</div><div>在线</div><div>下行速度(MB/s)</div><div>上行速度(KB/s)</div><div>总下行(MB)</div><div>总上行(KB)</div><div>启用</div>
        </div>
        <div class="row" v-for="p in peers" :key="p.pid">
          <div>{{ p.pid }}</div>
          <div>{{ p.is_online ? '是' : '否' }}</div>
          <div>{{ p.download_speed }}</div>
          <div>{{ p.upload_speed }}</div>
          <div>{{ p.total_download }}</div>
          <div>{{ p.total_upload }}</div>
          <div><ToggleSwitch :modelValue="p.is_enabled" @update:modelValue="v => change(p, v)" /></div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import ToggleSwitch from '@/components/ToggleSwitch.vue'
import { listPeers, getStats, togglePeer } from '@/services/peerApi'
export default {
  name: 'PeerPage',
  components: { ToggleSwitch },
  data(){ return { peers: [], stats: {}, timer: null } },
  async mounted(){
    await this.refresh()
    this.timer = setInterval(this.refresh, 1000)
  },
  unmounted(){ if (this.timer) clearInterval(this.timer) },
  methods: {
    async refresh(){ this.peers = await listPeers(); this.stats = await getStats() },
    async change(p, enabled){ await togglePeer(p.pid, enabled); p.is_enabled = enabled }
  }
}
</script>

<style scoped>
.top { display: flex; justify-content: flex-start; gap: 16px; margin-bottom: 12px; color: #333 }
.table { overflow-x: auto; }
.thead, .row { display: grid; grid-template-columns: 1.2fr .6fr 1.2fr 1.2fr 1.2fr 1.2fr .8fr; gap: 8px; padding: 8px 0; }
.thead { font-weight: bold; border-bottom: 2px solid var(--border-color) }
.row { border-bottom: 1px solid var(--border-color) }
</style>
