# Feelworld Communication Protocols for ESP32 Multitally

## 1. Connection Protocol
- **Command**: Connect
- **Hex Command**: `68`
- **Sub Command**: `66`
- **Message Format**: `<T|F><address><seq><cmd><dat1><dat2><dat3><dat4><sum>`
- **Example**: `<T00010068006600000000>`

## 2. Status Request Protocol
- **Command**: Statusnote
- **Hex Command**: `F1`
- **Sub Command**: `40`
- **Message Format**: `<T|F><address><seq><cmd><dat1><dat2><dat3><dat4><sum>`
- **Example**: `<T000100F14001000000>`

## 3. Camera States
- **States**: 
  - `live`: Camera is currently live
  - `preview`: Camera is in preview mode
  - `online`: Camera is connected but not live
  - `offline`: Camera is not connected

## 4. UDP Communication
- **Port**: 1000
- **Data Format**: Messages are sent as raw datagrams over UDP.

## 5. Error Handling
- Validate responses for sequence numbers and message formats.
