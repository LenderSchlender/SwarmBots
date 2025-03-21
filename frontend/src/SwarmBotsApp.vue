<script lang="ts"> // indicates that we are using typescript
export default {
  
}
</script>

<template>
  <div class="Router">
    <RouterLink to="/">Startseite </RouterLink>
    <RouterLink to="/daten">Daten </RouterLink>
    <br>
    <RouterView></RouterView>
  </div>

  <img src="./img/HTL_Logo.png" class="HTL_Logo">
</template>
  
<style lang="css">
  body
    {
      background-color: white;
    }
  .Router{
    position: absolute;
  }
  .HTL_Logo{
      position: absolute;
      margin-left: 720px;
    }
</style> 
