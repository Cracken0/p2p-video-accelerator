import api from './api'

export function listPeers() {
  return api.get('/peers').then(r => r.data)
}

export function getStats() {
  return api.get('/peers/stats').then(r => r.data)
}

export function togglePeer(pid, enabled) {
  return api.put(`/peers/${pid}/toggle`, { enabled }).then(r => r.data)
}
