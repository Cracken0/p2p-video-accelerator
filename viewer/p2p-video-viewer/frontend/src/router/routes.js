export default [
  {
    path: '/',
    redirect: '/index'
  },
  {
    path: '/index',
    name: 'Home',
    component: () => import('../views/HomePage.vue')
  },
  {
    path: '/search',
    name: 'Search',
    component: () => import('../views/SearchPage.vue')
  },
  {
    path: '/video',
    name: 'Video',
    component: () => import('../views/VideoPage.vue')
  },
  {
    path: '/settings',
    name: 'Settings',
    component: () => import('../views/SettingsPage.vue')
  },
  {
    path: '/peer',
    name: 'Peer',
    component: () => import('../views/PeerPage.vue')
  },
  {
    path: '/history',
    name: 'History',
    component: () => import('../views/HistoryPage.vue')
  }
]
