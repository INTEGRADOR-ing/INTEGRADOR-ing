# Detalles Técnicos del Proyecto

Este documento describe las herramientas, configuraciones y consideraciones técnicas utilizadas en el desarrollo del proyecto integrador.

---

## Interfaz de Usuario  

La interfaz de usuario fue desarrollada en MATLAB versión **2022b**, utilizando la herramienta **App Designer**.  
Esta interfaz permite controlar la máquina de tipo *pick and place* mediante comandos visuales, ofreciendo una experiencia amigable e intuitiva para el usuario.

---

## Programación de la Tarjeta STM  

Para la programación de la tarjeta STM se utilizó el entorno de desarrollo **Mbed Keil**, configurado para trabajar con una arquitectura basada en microcontroladores STM32.  

### Características de la Programación  

- **Lenguaje:** La programación en **Keil** se realizó en **C**, asegurando compatibilidad y optimización para microcontroladores de la familia STM32.  
- **Tarjeta Base:** La programación original fue diseñada específicamente para la tarjeta **Blue Pill**, una placa económica y versátil con un microcontrolador STM32F103C8.  

---

## Compatibilidad con otras Tarjetas  

El proyecto está diseñado para funcionar con una amplia gama de tarjetas de la familia STM32, incluyendo:  

- **Nucleo**  
- **Blue Pill**  
- **Black Pill**  

Sin embargo, al utilizar una tarjeta diferente a la **Blue Pill**, es importante considerar las siguientes configuraciones:  

1. **Conexiones Físicas del Serial:**  
   - Asegúrese de conectar correctamente los pines para la comunicación serial (USART).  
   - Verifique las configuraciones del pinout de la tarjeta específica para ubicar los pines TX y RX adecuados.  

2. **Pines de Conexión:**  
   - Revise y adapte las conexiones de control (GPIO, alimentación y señales de control) según el diseño eléctrico de la tarjeta seleccionada.  
   - Consulte el diagrama de pines de la tarjeta para evitar conflictos entre señales o interrupciones.

---

## Recomendaciones Adicionales  

- Al cambiar de tarjeta, realice pruebas iniciales con programas básicos para confirmar la correcta configuración del entorno y las conexiones físicas antes de cargar el programa completo.  
- Mantenga siempre actualizados los controladores (drivers) del microcontrolador STM32 para evitar problemas de compatibilidad con Mbed Keil.  

---

**Nota:** La adaptación a otras tarjetas puede requerir ajustes en el código fuente, dependiendo de las características y recursos específicos del microcontrolador.  

---

**Universidad ECCI - 2024**
