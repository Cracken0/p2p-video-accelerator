import axios from 'axios'

const api = axios.create({
  baseURL: process.env.VUE_APP_API_BASE || 'http://localhost:8000/api',
  timeout: 10000
})

export default api
