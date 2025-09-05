<template>
  <div class="search-bar">
    <input class="input" type="text" v-model="keyword" :placeholder="placeholder" @keyup.enter="onSearch" />
    <button class="btn" @click="onSearch">搜索</button>
  </div>
</template>

<script>
export default {
  name: 'SearchBar',
  props: { placeholder: { type: String, default: '搜索内容...' } },
  emits: ['search'],
  data() { return { keyword: '' } },
  methods: {
    onSearch() { this.$emit('search', this.keyword.trim()) }
  }
}
</script>

<style scoped>
.search-bar {
  display: flex;
  gap: 12px;
  align-items: center;
  justify-content: center;
}
.search-bar .input { flex: 1; max-width: 600px; height: 44px; }
.search-bar .btn { height: 44px; }
</style>
