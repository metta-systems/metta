incbin "grub/stage1"
incbin "grub/stage2"
align 512
incbin "kernel.bin"
align 512
db 'End of Image.'
