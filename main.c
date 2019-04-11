init main (void)
{
  /* USER CODE BEGIN 1 */
   QSPI_CommandTypeDef sCommand;
   uint32_t address = 10;
   uint16_t index;
   uint8_t step = 0;
  /* USER CODE END 1 */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
 // MX_IWDG1_Init();
  MX_TIM2_Init();
  MX_CRC_Init();
  MX_I2C2_Init();
  MX_QUADSPI_Init();
  MX_SPI2_Init();
  MX_USB_OTG_FS_USB_Init();
  MX_TIM5_Init();
 // MX_WWDG1_Init();
  MX_RTC_Init();
  MX_FMC_Init();
  MX_USART3_UART_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_DAC1_Init();
  MX_ADC2_Init();
  MX_ADC3_Init();
  MX_SPI1_Init();
  MX_TIM4_Init();
  MX_TIM15_Init();
  MX_I2C3_Init();
  MX_TIM8_Init();
  /* USER CODE BEGIN 2 */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode         = QSPI_SIOO_INST_EVERY_CMD;
  /* USER CODE END 2 */
  printf("\n\r ****QSPI Test Begin****");
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      
      switch(step)
      {
         case 0:
         {
            CmdCplt = 0;

            /* Initialize Reception buffer --------------------------------------- */
            for (index = 0; index <= BUFFERSIZE; index++)
            {
               aRxBuffer[index] = 0;
            }
            /* Enable write operations ------------------------------------------- */
            QSPI_WriteEnable(&hqspi);

            /* Erasing Sequence -------------------------------------------------- */
            sCommand.Instruction = SECTOR_ERASE_CMD;
            sCommand.AddressMode = QSPI_ADDRESS_1_LINE;
            sCommand.Address     = address;
            sCommand.DataMode    = QSPI_DATA_NONE;
            sCommand.DummyCycles = 0;
            if (HAL_QSPI_Command_IT(&hqspi, &sCommand) != HAL_OK)
            {
               Error_Handler();
            }
            step++;
            printf("\n\r QSPI Test : Erase sequence initiated ");
         }
         break;

         case 1:
         {
            if(CmdCplt != 0)
            {
               CmdCplt = 0;
               StatusMatch = 0;
               printf("\n\r QSPI Test : Erase Completed ");
               /* Configure automatic polling mode to wait for end of erase ------- */
               QSPI_AutoPollingMemReady(&hqspi);
 
               step++;
            }
         }
         break;
   
         case 2:
         {
            if(StatusMatch != 0)
            {
               StatusMatch = 0;
               TxCplt = 0;
   
               /* Enable write operations ----------------------------------------- */
               QSPI_WriteEnable(&hqspi);
                
                /* Added this delay to wait for QSPI return to ready state from busy state*/
                HAL_Delay(1000);
                
               /* Writing Sequence ------------------------------------------------ */
               sCommand.Instruction = QUAD_IN_FAST_PROG_CMD;
               sCommand.AddressMode = QSPI_ADDRESS_1_LINE;
               sCommand.DataMode    = QSPI_DATA_4_LINES;
               sCommand.NbData      = BUFFERSIZE;
      
               if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
               {
                  Error_Handler();
               }
      
               if (HAL_QSPI_Transmit_IT(&hqspi, aTxBuffer) != HAL_OK)
               {
                  Error_Handler();
               }
               step++;
               
               printf("\n\r QSPI Test : Write Sequence Initiated\n\r Transferred data : %s ", aTxBuffer);
            }
         }
         break;

         case 3:
         {
            if(TxCplt != 0)
            {
               TxCplt = 0;
               StatusMatch = 0;

               /* Configure automatic polling mode to wait for end of program ----- */
               QSPI_AutoPollingMemReady(&hqspi);
               step++;
               printf("\n\r QSPI Test : Write Completed");
            }
         }
         break;

         case 4:
         {
            if(StatusMatch != 0)
            {
               StatusMatch = 0;
               RxCplt = 0;

               /* Configure Volatile Configuration register (with new dummy cycles) */
               QSPI_DummyCyclesCfg(&hqspi);

               /* Reading Sequence ------------------------------------------------ */
               sCommand.Instruction = QUAD_OUT_FAST_READ_CMD;
               sCommand.DummyCycles = DUMMY_CLOCK_CYCLES_READ_QUAD;
               sCommand.Address = address -1;

               if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
               {
                  Error_Handler();
               }

               if (HAL_QSPI_Receive_IT(&hqspi, aRxBuffer) != HAL_OK)
               {
                  Error_Handler();
               }
               step++;
               printf("\n\r QSPI Test : Read sequence initiated ");
            }
         }
         break;

         case 5:
         {
            if (RxCplt != 0)
            {
               RxCplt = 0;

               /* Result comparison ----------------------------------------------- */
               for (index = 0; index < BUFFERSIZE; index++)
               {
                  if (aRxBuffer[index] != aTxBuffer[index])
                  {
                     printf("\n\r ERROR: Data mismatch ");
                     
                  }
               }
              
               //BSP_LED_Toggle(LED1);

               address += QSPI_PAGE_SIZE;
               if(address >= (QSPI_END_ADDR))
               {
                  address = 0;
               }
               step++;
               printf("\n\r QSPI Test : Read Completed \n\r Received data: %s  \n\r ****QSPI Test Completed**** ",aRxBuffer);
            }
         }
         break;
         case 6:
         {
           printf("\n\r Deinit QSPI");
           HAL_QSPI_DeInit(&hqspi);
           while(1);
            
         }
         break;
         default :
         {
            printf("\n\r QSPI Test Failed : Invaild step : %d", step); 
            while(1);
         }
    }
    /* USER CODE END WHILE */
    
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
  }


