import api from './api'

export function listHistory() {
  return api.get('/history').then(r => r.data)
}

export function addHistory(payload) {
  return api.post('/history', payload).then(r => r.data)
}

export function updateHistory(id, payload) {
  return api.put(`/history/${id}`, payload).then(r => r.data)
}

export function deleteHistory(id) {
  return api.delete(`/history/${id}`).then(r => r.data)
}
