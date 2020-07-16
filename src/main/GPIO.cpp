#include "GPIO.hpp"

int readPin(int pin) {
    if (pin < 16) {
        return GPIP(pin);
    } else if (pin == 16) {
        return GP16I & 0x01;
    }
    return 0;
}

void enablePin(int pin) { setPin(pin, LOW); /* Set state to LOW */ }
void disablePin(int pin) { setPin(pin, HIGH); /* Set state to HIGH */ }

void invertPin(int pin) {
    if (GPIP(pin))
        enablePin(pin);
    else
        disablePin(pin);
}

void setPin(int pin, int val) {
    if (pin < 16 && pin > -1) {
        if (val)
            GPOS = (1 << pin);
        else
            GPOC = (1 << pin);
    } else if (pin == 16) {
        if (val)
            GP16O |= 1;
        else
            GP16O &= ~1;
    }
}

void setOutput(int pin, int val) {
    setPin(pin, val);
    if (pin < 16) {
        GPF(pin) = GPFFS(GPFFS_GPIO(pin));      //Set mode to GPIO
        GPC(pin) = (GPC(pin) & (0xF << GPCI));  //SOURCE(GPIO) | DRIVER(NORMAL) | INT_TYPE(UNCHANGED) | WAKEUP_ENABLE(DISABLED)
        GPES = (1 << pin);                      //Enable
    } else if (pin == 16) {
        GPF16 = GP16FFS(GPFFS_GPIO(pin));  //Set mode to GPIO
        GPC16 = 0;
        GP16E |= 1;
    }
}