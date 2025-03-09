import { fileURLToPath, URL } from 'node:url'

import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'
import vueJsx from '@vitejs/plugin-vue-jsx'
import vueDevTools from 'vite-plugin-vue-devtools'
import { exec } from 'node:child_process'
import fs from "fs"
import path from "path"

// https://vite.dev/config/
export default defineConfig({
  plugins: [
    vue(),
    vueJsx(),
    vueDevTools(),
    {
      name: "vite-compile-protobufs",
      buildStart(options) {
        // Make sure output directory exists
        if (!fs.existsSync("./src/lib")) {
          console.log("Creating dir for compiled protobufs...")
          fs.mkdirSync("./src/lib")
        }

        // Make sure old versions have been disposed of
        fs.readdirSync("./src/lib").forEach(file => {
          fs.rmSync(path.join("./src/lib", file));
        });

        // Compile protobufs
        exec("npx protoc --proto_path=../proto --ts_out=./src/lib/ ../proto/*.proto", (error, stdout, stderr) => {
            if (error)
              console.error('\n'+error.message)
            if (stderr)
              console.error('\n'+stderr)
            if (stdout)
              console.log('\n'+stdout)
        })
        console.log("Compiled protobufs!")
      },
    }
  ],
  resolve: {
    alias: {
      '@': fileURLToPath(new URL('./src', import.meta.url))
    },
  },
})
