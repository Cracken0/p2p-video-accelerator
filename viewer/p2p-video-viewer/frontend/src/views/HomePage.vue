<template>
  <div class="container">
    <div style="height: 20vh"></div>
    <div class="card" style="text-align:center; padding: 40px 20px;">
      <SearchBar :placeholder="'输入关键字搜索'" @search="goSearch" />
      <div style="margin-top: 20px; display:flex; gap:12px; justify-content:center;">
        <router-link class="btn" to="/settings">设置</router-link>
        <router-link class="btn" to="/peer">peer管理</router-link>
        <router-link class="btn" to="/history">历史记录</router-link>
      </div>
    </div>
  </div>
</template>

<script>
import SearchBar from '@/components/SearchBar.vue'
export default {
  name: 'HomePage',
  components: { SearchBar },
  methods: {
    goSearch(keyword){
      if (!keyword) return
      this.$router.push({ path: '/search', query: { query: keyword } })
    }
  }
}
</script>
