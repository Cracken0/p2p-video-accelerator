<template>
  <div class="status-bar">
    <span>节点数: {{ stats.total_peers || 0 }}</span>
    <span>  下行(MB/s): {{ (stats.total_download_speed || 0).toFixed(1) }}</span>
    <span>  上行(KB/s): {{ (stats.total_upload_speed || 0).toFixed(1) }}</span>
  </div>
</template>

<script>
export default {
  name: 'StatusBar',
  props: { stats: { type: Object, default: () => ({}) } }
}
</script>

<style scoped>
.status-bar { display: flex; gap: 16px; justify-content: flex-end; color: #555; }
.status-bar span + span { margin-left: 8px; }
</style>
