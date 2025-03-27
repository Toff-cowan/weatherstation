// Composables
import { createRouter, createWebHistory } from 'vue-router'

const routes = [
  { path: '/', name: 'Home', component: () => import('@/views/Home.vue'), meta:{ transition: 'fade'} },
  { path: '/Live', name: 'Live', component: () => import('@/views/Live.vue'), meta:{ transition: 'fade'} },
  { path: '/Analysis', name: 'Analysis', component: () => import('@/views/Analysis.vue'), meta:{ transition: 'fade'} }
  // ADD NEW ROUTES HERE
  
]

const router = createRouter({
  history: createWebHistory(process.env.BASE_URL),
  routes,
})

export default router
