import api from './api'

export function getVideo(rid) {
  return api.get(`/videos/${rid}`).then(r => r.data)
}

export function getStream(rid) {
  return api.get(`/videos/${rid}/stream`).then(r => r.data)
}

export function getStatus(rid) {
  return api.get(`/videos/${rid}/status`).then(r => r.data)
}

export function updateStatus(rid, status) {
  return api.put(`/videos/${rid}/status`, status).then(r => r.data)
}
