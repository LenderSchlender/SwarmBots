import './assets/main.css'

import { createApp } from 'vue'
import { createPinia } from 'pinia'

import SwarmBotsApp from './SwarmBotsApp.vue'
import router from './router'

import VueApexCharts from "vue3-apexcharts";

const app = createApp(SwarmBotsApp)

app.use(createPinia())
app.use(router)
app.use(VueApexCharts);

app.mount('#app')