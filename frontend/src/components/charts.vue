<script lang="ts">
import VueApexCharts from 'vue3-apexcharts'
import pulses from './daten.vue'

export default {
  name: 'encoder_Chart',
  components: {
    apexcharts: VueApexCharts,
  },
  data: function() {
    return {
      chartOptions: {
        chart: {
          id: 'encoder_bar'
        },
        xaxis: {
          title: {
            text: 't [s]',  
          },
          categories: [1, 2, 3, 4, 5, 6, 7, 8]
        },
        yaxis: {
          title: {
            text: 'v [m/s]',
          },
        }
      },
      series: [{
        name: 'series-1',
        data: [pulses] // Beispielwerte [0,2,5,2,1,3,4]
      }]
    }
  },
}
</script>

<template>
  <div class="example">
    <apexcharts width="500" type="line" :options="chartOptions" :series="series"></apexcharts>
  </div>
</template>

<style lang="css">
  .example{
    margin-top: 1rem;
  }
</style>
