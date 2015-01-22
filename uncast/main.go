package main

import "encoding/hex"
import "crypto/aes"
import "crypto/cipher"
import "io/ioutil"

func main() {
	key_raw := "Wii U ancast key here"
	key, _ := hex.DecodeString(key_raw)
	block, _ := aes.NewCipher(key)
	var allzeroes [16]byte
	cbc := cipher.NewCBCDecrypter(block, allzeroes[:])
	data, _ := ioutil.ReadFile("kernel.img")
	view := data[0x100:]
	cbc.CryptBlocks(view, view)
	ioutil.WriteFile("kernel.out", data, 0)
}