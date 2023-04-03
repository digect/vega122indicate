# Makefile for AVR C++ projects
# From: https://gist.github.com/rynr/72734da4b8c7b962aa65

# ----- Update the settings of your project here -----

# Hardware
MCU     = atmega88p # see `make show-mcu`
OSC     = 8000000UL
PROJECT = disunit

# ----- These configurations are quite likely not to be changed -----

# Binaries
GCC     = avr-gcc
G++     = avr-g++

AVRDUDE = avrdude
AVRDUDE_MCPU = m88p
AVRDUDE_PROG = usbasp
AVRDUDE_PORT = usb

ifeq ($(OS),Windows_NT)
    RM      = del /s /q
else
    RM      = rm -f
endif

#EEPROM_CMD = ./serial/version.sh

# Files
EXT_C   = c
EXT_C++ = cpp

# ----- No changes should be necessary below this line -----

OBJECTS = \
	$(patsubst %.$(EXT_C),%.o,$(wildcard *.$(EXT_C))) \
	$(patsubst %.$(EXT_C++),%.o,$(wildcard *.$(EXT_C++))) \

# TODO explain these flags, make them configurable
CFLAGS = $(INC)
CFLAGS += -Os
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -Wall -Wstrict-prototypes
CFLAGS += -DF_OSC=$(OSC)
CFLAGS += -mmcu=$(MCU)

C++FLAGS = $(INC)
C++FLAGS += -Os
C++FLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
C++FLAGS += -Wall
C++FLAGS += -DF_OSC=$(OSC)
C++FLAGS += -mmcu=$(MCU)

#ASMFLAGS = $(INC)
#ASMFLAGS += -Os
#ASMFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
#ASMFLAGS += -Wall -Wstrict-prototypes
#ASMFLAGS += -DF_OSC=$(OSC)
#ASMFLAGS += -x assembler-with-cpp
#ASMFLAGS += -mmcu=$(MCU)


#default: $(PROJECT).elf
all: $(PROJECT).elf
	echo $(OBJECTS)

%.elf: $(OBJECTS)
	$(GCC) $(CFLAGS) $(OBJECTS) --output $@ $(LDFLAGS)
	avr-objcopy -j .text -j .data -O ihex $@ $(PROJECT).hex
	avr-size  $@


%.o : %.$(EXT_C)
	$(GCC) $< $(CFLAGS) -c -o $@

%.o : %.$(EXT_C++)
	$(G++) $< $(C++FLAGS) -c -o $@

#%.o : %.$(EXT_ASM)
#	$(G++) $< $(ASMFLAGS) -c -o $@

clean:
	$(RM) $(PROJECT).elf  $(PROJECT).hex  $(OBJECTS)
prog:
#	$(EEPROM_CMD)
	$(AVRDUDE) -p $(AVRDUDE_MCPU) -c $(AVRDUDE_PROG) -P $(AVRDUDE_PORT) -v -U flash:w:$(PROJECT).hex 
flash:
	$(AVRDUDE) -p $(AVRDUDE_MCPU) -c $(AVRDUDE_PROG) -P $(AVRDUDE_PORT) -v -U flash:w:$(PROJECT).hex


help:
	@echo "usage:"
	@echo "  make <target>"
	@echo ""
	@echo "targets:"
	@echo "  clean     Remove any non-source files"
	@echo "  config    Shows the current configuration"
	@echo "  help      Shows this help"

config:
	@echo "configuration:"
	@echo ""
	@echo "Binaries for:"
	@echo "  C compiler:   $(GCC)"
	@echo "  C++ compiler: $(G++)"
	@echo "  Programmer:   $(AVRDUDE)"
	@echo "  remove file:  $(RM)"
	@echo ""
	@echo "Hardware settings:"
	@echo "  MCU: $(MCU)"
	@echo "  OSC: $(OSC)"
	@echo ""
	@echo "Defaults:"
	@echo "  C-files:   *.$(EXT_C)"
	@echo "  C++-files: *.$(EXT_C++)"
	@echo "  ASM-files: *.$(EXT_ASM)"
