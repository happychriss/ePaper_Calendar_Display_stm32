
void FSMC_SRAM_Init();
void FSMC_SRAM_WriteBuffer(uint16_t* pBuffer, uint32_t WriteAddr, uint32_t NumHalfwordToWrite);
void FSMC_SRAM_ReadBuffer(uint16_t* pBuffer, uint32_t ReadAddr, uint32_t NumHalfwordToRead);
void FSMC_SRAM_FillBuffer(uint16_t value, uint32_t WriteAddr, uint32_t NumHalfwordToWrite);


