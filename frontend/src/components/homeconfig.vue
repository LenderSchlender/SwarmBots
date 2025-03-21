<template>
  <div class="item">
    <i>
      <slot name="icon"></slot>
    </i>
    <div class="details">
      <h3>
        <slot name="heading"></slot>
      </h3>
      <slot></slot>
    </div>
  </div>
</template>


<style scoped>
/* nur Icons und Text */
.item {
  margin-top: 2rem;
  display: flex;
  position: relative;
}

.details {
  flex: 1;
  margin-left: 1rem;
}

i {
  display: flex;
  place-items: center;
  place-content: center;
  width: 32px;
  height: 32px;

  color: var(--color-text);
}

h3 {
  font-size: 1.7rem;
  font-weight: 500;
  margin-bottom: 0.2rem;
  color: black;
}

@media (min-width: 1024px) {
  .item {
    margin-top: 0;
    padding: 0.4rem 0 1rem 1rem;
  }

  i {
    top: 0.5rem;
    left: -26px;
    position: absolute;
    border: 1px solid var(--color-border);
    background: white;
    border-radius: 8px;
    width: 50px;
    height: 50px;
  }

  .item:first-of-type:before {
    content: ' ';
    border-left: 1px solid var(--color-border);
    position: absolute;
    left: 0;
    bottom: 0rem;
    height: 7.8rem;
  }

  .item:last-of-type:after {
    content: ' ';
    border-left: 1px solid var(--color-border);
    position: absolute;
    left: 0;
    bottom: 10rem;
    height: 2.5rem;
  }
}
</style>

<script lang="ts">
export default {
  name: "HomeConfig"
}
</script>