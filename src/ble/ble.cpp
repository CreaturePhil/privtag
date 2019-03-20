#include <SPI.h>
#include "../include/ble.h"
#include "../include/lib_aci.h"
#include "../include/aci_setup.h"
#include "../include/services.h"
#include "../include/link_loss.h"


#ifdef SERVICES_PIPE_TYPE_MAPPING_CONTENT
static services_pipe_type_mapping_t
services_pipe_type_mapping[NUMBER_OF_PIPES] = SERVICES_PIPE_TYPE_MAPPING_CONTENT;
#else
#define NUMBER_OF_PIPES 0
static services_pipe_type_mapping_t * services_pipe_type_mapping = NULL;
#endif

static const hal_aci_data_t setup_msgs[NB_SETUP_MESSAGES] = SETUP_MESSAGES_CONTENT;

static struct aci_state_t aci_state;

// Temporary buffers for sending ACI commands
static hal_aci_evt_t  aci_data;
static hal_aci_data_t aci_cmd;

// Timing change state variable
static bool timing_change_done = false;

/* Define how assert should function in the BLE library */
void __ble_assert(const char *file, uint16_t line)
{
  SerialUSB.print("ERROR ");
  SerialUSB.print(file);
  SerialUSB.print(": ");
  SerialUSB.print(line);
  SerialUSB.print("\n");
  while(1);
}

void ble_setup() {
  SerialUSB.println("BLEsetup()");
  /**
  Point ACI data structures to the the setup data that the nRFgo studio generated for the nRF8001
  */
  if (NULL != services_pipe_type_mapping)
  {
    aci_state.aci_setup_info.services_pipe_type_mapping = &services_pipe_type_mapping[0];
  }
  else
  {
    aci_state.aci_setup_info.services_pipe_type_mapping = NULL;
  }
  aci_state.aci_setup_info.number_of_pipes    = NUMBER_OF_PIPES;
  aci_state.aci_setup_info.setup_msgs         = (hal_aci_data_t*) setup_msgs;
  aci_state.aci_setup_info.num_setup_msgs     = NB_SETUP_MESSAGES;

  //Tell the ACI library, the MCU to nRF8001 pin connections
  aci_state.aci_pins.board_name = BOARD_DEFAULT; //See board.h for details
  aci_state.aci_pins.reqn_pin   = 10;
  aci_state.aci_pins.rdyn_pin   = 2;
  aci_state.aci_pins.mosi_pin   = MOSI;
  aci_state.aci_pins.miso_pin   = MISO;
  aci_state.aci_pins.sck_pin    = SCK;

  aci_state.aci_pins.spi_clock_divider      = 48;

  aci_state.aci_pins.reset_pin              = 9; //4 for Nordic board, UNUSED for REDBEARLABS
  aci_state.aci_pins.active_pin             = UNUSED;
  aci_state.aci_pins.optional_chip_sel_pin  = UNUSED;

  aci_state.aci_pins.interface_is_interrupt = false;
  aci_state.aci_pins.interrupt_number       = 1;

  //We reset the nRF8001 here by toggling the RESET line connected to the nRF8001
  //and initialize the data structures required to setup the nRF8001
  //The second parameter is for turning debug printing on for the ACI Commands and Events so they be printed on the SerialUSB
  lib_aci_init(&aci_state, true);
  aci_state.bonded = ACI_BOND_STATUS_FAILED;
}

void alert_level_print(alert_level_t level)
{
  switch (level)
  {
    case ALERT_LEVEL_NO_ALERT:
      SerialUSB.println(F("NO_ALERT"));
      break;

    case ALERT_LEVEL_MILD_ALERT:
      SerialUSB.println(F("MILD_ALERT"));
      break;

    case ALERT_LEVEL_HIGH_ALERT:
      SerialUSB.println(F("HIGH_ALERT"));
      break;
  }
}

void immediate_alert_hook(alert_level_t level)
{
  SerialUSB.println(F("Immediate Alert: Alert level = "));
  alert_level_print(level);
}

void link_loss_alert_hook(alert_level_t level)
{
  SerialUSB.println(F("Link Loss Alert: Alert level = "));
  alert_level_print(level);
}

void ble_loop()
{
  static bool setup_required = false;

  // We enter the if statement only when there is a ACI event available to be processed
  if (lib_aci_event_get(&aci_state, &aci_data))
  {
    aci_evt_t * aci_evt;

    aci_evt = &aci_data.evt;
    switch(aci_evt->evt_opcode)
    {
      /**
      As soon as you reset the nRF8001 you will get an ACI Device Started Event
      */
      case ACI_EVT_DEVICE_STARTED:
      {
        aci_state.data_credit_total = aci_evt->params.device_started.credit_available;
        switch(aci_evt->params.device_started.device_mode)
        {
          case ACI_DEVICE_SETUP:
            /**
            When the device is in the setup mode
            */
            SerialUSB.println(F("Evt Device Started: Setup"));
            setup_required = true;
            break;

          case ACI_DEVICE_STANDBY:
            SerialUSB.println(F("Evt Device Started: Standby"));
            if (aci_evt->params.device_started.hw_error)
            {
              delay(20); //Magic number used to make sure the HW error event is handled correctly.
            }
            else
            {
              // TODO load a previous bond from persistant storage
              
              // Start bonding as all proximity devices need to be bonded to be usable
              if (ACI_BOND_STATUS_SUCCESS != aci_state.bonded)
              {
                lib_aci_bond(180/* in seconds */, 0x0050 /* advertising interval 50ms*/);
                SerialUSB.println(F("No Bond present in EEPROM."));
                SerialUSB.println(F("Advertising started : Waiting to be connected and bonded"));
              }
              else
              {
                //connect to an already bonded device
                //Use lib_aci_direct_connect for faster re-connections with PC, not recommended to use with iOS/OS X
                lib_aci_connect(100/* in seconds */, 0x0020 /* advertising interval 20ms*/);
                SerialUSB.println(F("Already bonded : Advertising started : Waiting to be connected"));
              }
            }
            break;
        }
      }
        break; //ACI Device Started Event

      case ACI_EVT_CMD_RSP:
        //If an ACI command response event comes with an error -> stop
        if (ACI_STATUS_SUCCESS != aci_evt->params.cmd_rsp.cmd_status)
        {
          //ACI ReadDynamicData and ACI WriteDynamicData will have status codes of
          //TRANSACTION_CONTINUE and TRANSACTION_COMPLETE
          //all other ACI commands will have status code of ACI_STATUS_SCUCCESS for a successful command
          SerialUSB.print(F("ACI Command "));
          SerialUSB.println(aci_evt->params.cmd_rsp.cmd_opcode, HEX);
          SerialUSB.print(F("Evt Cmd respone: Status "));
          SerialUSB.println(aci_evt->params.cmd_rsp.cmd_status, HEX);
        }
        if (ACI_CMD_GET_DEVICE_VERSION == aci_evt->params.cmd_rsp.cmd_opcode)
        {
          //Store the version and configuration information of the nRF8001 in the Hardware Revision String Characteristic
          lib_aci_set_local_data(&aci_state, PIPE_DEVICE_INFORMATION_HARDWARE_REVISION_STRING_SET,
            (uint8_t *)&(aci_evt->params.cmd_rsp.params.get_device_version), sizeof(aci_evt_cmd_rsp_params_get_device_version_t));
        }
        break;

      case ACI_EVT_CONNECTED:
        SerialUSB.println(F("Evt Connected"));
        aci_state.data_credit_available = aci_state.data_credit_total;
        timing_change_done = false;
        /*
        Get the device version of the nRF8001 and store it in the Hardware Revision String
        */
        lib_aci_device_version();
        break;

      case ACI_EVT_BOND_STATUS:
        aci_state.bonded = aci_evt->params.bond_status.status_code;
        break;

      case ACI_EVT_PIPE_STATUS:
        SerialUSB.println(F("Evt Pipe Status"));
        //Link is encrypted when the PIPE_LINK_LOSS_ALERT_ALERT_LEVEL_RX_ACK_AUTO is available
        if ((false == timing_change_done) &&
          lib_aci_is_pipe_available(&aci_state, PIPE_LINK_LOSS_ALERT_ALERT_LEVEL_RX_ACK_AUTO))
        {
          lib_aci_change_timing_GAP_PPCP(); // change the timing on the link as specified in the nRFgo studio -> nRF8001 conf. -> GAP.
                                            // Used to increase or decrease bandwidth
          timing_change_done = true;
        }
        // The pipe will be available only in an encrpyted link to the phone
        if ((ACI_BOND_STATUS_SUCCESS == aci_state.bonded) &&
              (lib_aci_is_pipe_available(&aci_state, PIPE_LINK_LOSS_ALERT_ALERT_LEVEL_RX_ACK_AUTO)) &&
              (lib_aci_is_pipe_available(&aci_state, PIPE_IMMEDIATE_ALERT_ALERT_LEVEL_RX)))
        {
          //Note: This may be called multiple times after the Arduino has connected to the right phone
          SerialUSB.println(F("phone Detected."));
          SerialUSB.println(F("Do more stuff here. when your phone is detected"));
        }
        break;

      case ACI_EVT_TIMING:
        SerialUSB.println(F("Evt link connection interval changed"));
        //Disconnect as soon as we are bonded and required pipes are available
        //This is used to store the bonding info on disconnect and then re-connect to verify the bond
        /* TODO maybe we still need this
         * if((ACI_BOND_STATUS_SUCCESS == aci_state.bonded) &&
           (true == bonded_first_time) &&
           (GAP_PPCP_MAX_CONN_INT >= aci_state.connection_interval) &&
           (GAP_PPCP_MIN_CONN_INT <= aci_state.connection_interval) && //Timing change already done: Provide time for the the peer to finish
           (lib_aci_is_pipe_available(&aci_state, PIPE_LINK_LOSS_ALERT_ALERT_LEVEL_RX_ACK_AUTO)) &&
           (lib_aci_is_pipe_available(&aci_state, PIPE_IMMEDIATE_ALERT_ALERT_LEVEL_RX)))
         {
           lib_aci_disconnect(&aci_state, ACI_REASON_TERMINATE);
         }*/
          break;

      case ACI_EVT_DISCONNECTED:
        SerialUSB.println(F("Evt Disconnected. Link Lost or Advertising timed out"));
        if (ACI_BOND_STATUS_SUCCESS == aci_state.bonded)
        {
          if (ACI_STATUS_EXTENDED == aci_evt->params.disconnected.aci_status) //Link was disconnected
          {
            // TODO Read and store bonding information
            
            if (0x24 == aci_evt->params.disconnected.btle_status)
            {
              //The error code appears when phone or Arduino has deleted the pairing/bonding information.
              //The Arduino stores the bonding information in EEPROM, which is deleted only by
              // the user action of connecting pin 6 to 3.3v and then followed by a reset.
              //While deleting bonding information delete on the Arduino and on the phone.
              SerialUSB.println(F("phone/Arduino has deleted the bonding/pairing information"));
            }

            proximity_disconect_evt_rcvd (aci_evt->params.disconnected.btle_status);
          }
          lib_aci_connect(180/* in seconds */, 0x0100 /* advertising interval 100ms*/);
          SerialUSB.println(F("Using existing bond stored in EEPROM."));
          SerialUSB.println(F("   To delete the bond stored in EEPROM, connect Pin 6 to 3.3v and Reset."));
          SerialUSB.println(F("   Make sure that the bond on the phone/PC is deleted as well."));
          SerialUSB.println(F("Advertising started. Connecting."));
        }
        else
        {
          //There is no existing bond. Try to bond.
          lib_aci_bond(180/* in seconds */, 0x0050 /* advertising interval 50ms*/);
          SerialUSB.println(F("Advertising started. Bonding."));
        }
        break;

      case ACI_EVT_DATA_RECEIVED:
        SerialUSB.print(F("Pipe #"));
        SerialUSB.print(aci_evt->params.data_received.rx_data.pipe_number, DEC);
        SerialUSB.print(F("-> "));
        SerialUSB.println(aci_evt->params.data_received.rx_data.aci_data[0], DEC);
        link_loss_pipes_updated_evt_rcvd(aci_evt->params.data_received.rx_data.pipe_number,
                                         &aci_evt->params.data_received.rx_data.aci_data[0]);
        break;

      case ACI_EVT_DATA_CREDIT:
        aci_state.data_credit_available = aci_state.data_credit_available + aci_evt->params.data_credit.credit;
        break;

      case ACI_EVT_PIPE_ERROR:
        //See the appendix in the nRF8001 Product Specication for details on the error codes
        SerialUSB.print(F("ACI Evt Pipe Error: Pipe #:"));
        SerialUSB.print(aci_evt->params.pipe_error.pipe_number, DEC);
        SerialUSB.print(F("  Pipe Error Code: 0x"));
        SerialUSB.println(aci_evt->params.pipe_error.error_code, HEX);

        //Increment the credit available as the data packet was not sent.
        //The pipe error also represents the Attribute protocol Error Response sent from the peer and that should not be counted
        //for the credit.
        if (ACI_STATUS_ERROR_PEER_ATT_ERROR != aci_evt->params.pipe_error.error_code)
        {
          aci_state.data_credit_available++;
        }
        break;

      case ACI_EVT_HW_ERROR:
        SerialUSB.print(F("HW error: "));
        SerialUSB.println(aci_evt->params.hw_error.line_num, DEC);

        for(uint8_t counter = 0; counter <= (aci_evt->len - 3); counter++)
        {
        SerialUSB.write(aci_evt->params.hw_error.file_name[counter]); //uint8_t file_name[20];
        }
        SerialUSB.println();

        // TODO load a previous bond from persistant storage
        
        // Start bonding as all proximity devices need to be bonded to be usable
        if (ACI_BOND_STATUS_SUCCESS != aci_state.bonded)
        {
          lib_aci_bond(180/* in seconds */, 0x0050 /* advertising interval 50ms*/);
          SerialUSB.println(F("No Bond present in EEPROM."));
          SerialUSB.println(F("Advertising started : Waiting to be connected and bonded"));
        }
        else
        {
          //connect to an already bonded device
          //Use lib_aci_direct_connect for faster re-connections with PC, not recommended to use with iOS/OS X
          lib_aci_connect(100/* in seconds */, 0x0020 /* advertising interval 20ms*/);
          SerialUSB.println(F("Already bonded : Advertising started : Waiting to be connected"));
        }
        break;

    }
  }
  else
  {
    //SerialUSB.println(F("No ACI Events available"));
    // No event in the ACI Event queue and if there is no event in the ACI command queue the arduino can go to sleep
    // Arduino can go to sleep now
    // Wakeup from sleep from the RDYN line
  }
  
  /* setup_required is set to true when the device starts up and enters setup mode.
   * It indicates that do_aci_setup() should be called. The flag should be cleared if
   * do_aci_setup() returns ACI_STATUS_TRANSACTION_COMPLETE.
   */
  if(setup_required)
  {
    if (SETUP_SUCCESS == do_aci_setup(&aci_state))
    {
      setup_required = false;
    }
  }
}
