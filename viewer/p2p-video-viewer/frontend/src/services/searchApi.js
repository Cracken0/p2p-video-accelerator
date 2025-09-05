import api from './api'

export function search(query) {
  return api.get('/search', { params: { query } }).then(r => r.data)
}
