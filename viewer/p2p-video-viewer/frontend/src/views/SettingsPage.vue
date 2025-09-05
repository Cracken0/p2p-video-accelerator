<template>
  <div class="container">
    <div class="card">
      <div class="row"><span>开启p2p服务</span><ToggleSwitch v-model="form.p2p_service_enabled" /></div>
      <div class="row"><span>本地缓存上限(MB)</span><NumberInput v-model="form.local_cache_limit" /></div>
      <div class="row"><span>上传速度上限(KB/s)</span><NumberInput v-model="form.upload_speed_limit" /></div>
      <div class="row"><span>多线程数量</span><NumberInput v-model="form.thread_count" :integer="true" /></div>
      <div style="text-align:right; margin-top: 12px;"><button class="btn" @click="save">保存</button></div>
    </div>
  </div>
</template>

<script>
import ToggleSwitch from '@/components/ToggleSwitch.vue'
import NumberInput from '@/components/NumberInput.vue'
import { getSettings, updateSettings } from '@/services/settingsApi'

export default {
  name: 'SettingsPage',
  components: { ToggleSwitch, NumberInput },
  data(){ return { form: { p2p_service_enabled: true, local_cache_limit: '', upload_speed_limit: '', thread_count: '' } } },
  async mounted(){ this.form = await getSettings() },
  methods: {
    async save(){ this.form = await updateSettings(this.form) }
  }
}
</script>

<style scoped>
.row { display: flex; justify-content: space-between; align-items: center; padding: 10px 0; border-bottom: 1px solid var(--border-color) }
.row span { color: #333 }
</style>
