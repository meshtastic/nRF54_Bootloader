/*
 * Minimal ble_l2cap.h stub for S145.
 * S145 does not expose an L2CAP API, but the vendored DFU transport
 * code references BLE_L2CAP_MTU_DEF for buffer sizing.
 */

#ifndef BLE_L2CAP_H__
#define BLE_L2CAP_H__

/* Default ATT MTU minus 3-byte ATT header = 20 bytes payload.
 * This matches the legacy SoftDevice default. */
#ifndef BLE_L2CAP_MTU_DEF
#define BLE_L2CAP_MTU_DEF  23
#endif

#endif /* BLE_L2CAP_H__ */
