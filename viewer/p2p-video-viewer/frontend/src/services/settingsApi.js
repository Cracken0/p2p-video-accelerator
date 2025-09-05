import api from './api'

export function getSettings() {
  return api.get('/settings').then(r => r.data)
}

export function updateSettings(payload) {
  return api.put('/settings', payload).then(r => r.data)
}
