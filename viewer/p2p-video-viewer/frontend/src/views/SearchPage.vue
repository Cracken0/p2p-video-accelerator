<template>
  <div class="container">
    <div class="card" style="padding: 16px; position: sticky; top: 0; z-index: 1;">
      <SearchBar :placeholder="'输入关键字重新搜索'" @search="doSearch" />
    </div>
    <div>
      <div v-for="item in results" :key="item.rid" class="card result-item" @click="openVideo(item)">
        <div class="left">
          <div class="title">{{ item.title }}</div>
          <div class="desc">{{ item.description }}</div>
        </div>
        <img class="thumb" :src="item.thumbnail_url" alt="thumb" />
      </div>
    </div>
  </div>
</template>

<script>
import SearchBar from '@/components/SearchBar.vue'
import { search } from '@/services/searchApi'
export default {
  name: 'SearchPage',
  components: { SearchBar },
  data(){ return { results: [] } },
  mounted(){ this.doSearch(this.$route.query.query || '') },
  methods: {
    async doSearch(q){ this.results = await search(q) },
    openVideo(item){ this.$router.push({ path: '/video', query: { rid: item.rid } }) }
  }
}
</script>

<style scoped>
.result-item { display: flex; justify-content: space-between; align-items: center; cursor:pointer; }
.result-item .left { flex: 1; padding-right: 16px; }
.result-item .title { font-weight: bold; font-size: 18px; margin-bottom: 8px; }
.result-item .desc { color: #666; }
.result-item .thumb { width: 100px; height: 100px; object-fit: cover; border-radius: 8px; }
</style>
