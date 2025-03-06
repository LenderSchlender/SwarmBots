<script>
import VueApexCharts from 'vue3-apexcharts'

export default {
  name: 'Chart',
  components: {
    apexcharts: VueApexCharts,
  },
  data: function() {
    return {
      chartOptions: {
        chart: {
          id: 'basic-bar'
        },
        xaxis: {
          categories: [1, 2, 3, 4, 5, 6, 7, 8]
        }
      },
      series: [{
        name: 'series-1',
        data: [0, 4, 1, 8, 4, 5, 6, 6]
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
