PROJ=test
TARGET_ARCH=-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb # specs might set these
CC=arm-none-eabi-gcc
CFLAGS= -Wall


OBJS=ivt.o startup.o main.o system_stm32f4xx.o
AS=arm-none-eabi-as
LD=arm-none-eabi-ld
ASFLAGS=
LDFLAGS=
LDLIBS=

vpath %.c src

.PHONY: all
all: $(PROJ).bin

%.o: %.s
	$(CC) -x assembler-with-cpp -c -O0 -Wall $(TARGET_ARCH) --specs=nosys.specs -o $@ $<

%.o: %.c
	$(CC) -c -O0 -Wall $(TARGET_ARCH) --specs=nosys.specs -nostdlib -lgcc -I./inc -o $@ $<

$(PROJ).elf: $(OBJS)
	$(CC) $^ $(TARGET_ARCH) --specs=nosys.specs -nostdlib -lgcc -T linker.ld -Wl,-Map test.map -o $@

$(PROJ).bin: $(PROJ).elf
	arm-none-eabi-objcopy -S -O binary $< $@

.PHONY: flash
flash: $(PROJ).bin
	st-flash write $(PROJ).bin 0x8000000

.PHONY: clean
clean:
	rm -f $(OBJS)
	rm -f $(PROJ).elf