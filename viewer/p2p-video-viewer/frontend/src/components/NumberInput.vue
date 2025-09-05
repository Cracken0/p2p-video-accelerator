<template>
  <input class="input" :value="local" @input="onInput" :placeholder="placeholder" />
</template>

<script>
export default {
  name: 'NumberInput',
  props: { modelValue: [Number, String], placeholder: String, integer: Boolean },
  emits: ['update:modelValue'],
  data() { return { local: this.modelValue } },
  watch: { modelValue(v){ this.local = v } },
  methods: {
    onInput(e){
      const val = e.target.value
      const re = this.integer ? /^\d*$/ : /^\d*(?:\.\d*)?$/
      if (val === '' || re.test(val)) {
        this.local = val
        const num = val === '' ? '' : (this.integer ? parseInt(val || '0') : parseFloat(val || '0'))
        this.$emit('update:modelValue', isNaN(num) ? '' : num)
      } else {
        e.target.value = this.local
      }
    }
  }
}
</script>
