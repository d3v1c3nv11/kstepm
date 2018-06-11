portb = (readl(pb_base + PB_CONFIG0 + (PCHx > 7 ) * 4 + (PCHx > 15) * 4) & ~(0x0f << (PCHx & 0x07) * 4)) | (1 << (PCHx & 0x07) * 4);
	writel(portb, pb_base + PB_CONFIG0 + (PCHx > 7 ) * 4 + (PCHx > 15) * 4);
	portb = (readl(pb_base + PB_CONFIG0 + (DCHx > 7 ) * 4 + (DCHx > 15) * 4) & ~(0x0f << (DCHx & 0x07) * 4)) | (1 << (DCHx & 0x07) * 4);
	writel(portb, pb_base + PB_CONFIG0 + (DCHx > 7 ) * 4 + (DCHx > 15) * 4);	
	
	
	portb = (readl(pb_base + PB_CONFIG0 + (PCHy1 > 7 )*4 + (PCHy1 > 15) * 4) & ~(0x0f << (PCHy1 & 0x07) * 4)) | (1 << (PCHy1 & 0x07) * 4);
	writel(portb, pb_base + PB_CONFIG0 + (PCHy1 > 7 )*4 + (PCHy1 > 15) * 4);
	portb = (readl(pb_base + PB_CONFIG0 + (DCHy1 > 7 )*4 + (DCHy1 > 15) * 4) & ~(0x0f << (DCHy1 & 0x07) * 4)) | (1 << (DCHy1 & 0x07) * 4);
	writel(portb, pb_base + PB_CONFIG0 + (DCHy1 > 7 )*4 + (DCHy1 > 15) * 4);		

	portb = (readl(pb_base + PB_CONFIG0 + (PCHz > 7 )*4 + (PCHz > 15) * 4) & ~(0x0f << (PCHz & 0x07) * 4)) | (1 << (PCHz & 0x07) * 4);
	writel(portb, pb_base + PB_CONFIG0 + (PCHz > 7 )*4 + (PCHz > 15) * 4);
	portb = (readl(pb_base + PB_CONFIG0 + (DCHz > 7 )*4 + (DCHz > 15) * 4) & ~(0x0f << (DCHz & 0x07) * 4)) | (1 << (DCHz & 0x07) * 4);
	writel(portb, pb_base + PB_CONFIG0 + (DCHz > 7 )*4 + (DCHz > 15) * 4);	
	
	
	portb = (readl(pb_base + PB_CONFIG0 + (PCHa > 7 )*4 + (PCHa > 15) * 4) & ~(0x0f << (PCHa & 0x07) * 4)) | (1 << (PCHa & 0x07) * 4);
	writel(portb, pb_base + PB_CONFIG0 + (PCHa > 7 )*4 + (PCHa > 15) * 4);
	portb = (readl(pb_base + PB_CONFIG0 + (DCHa > 7 )*4 + (DCHa > 15) * 4) & ~(0x0f << (DCHa & 0x07) * 4)) | (1 << (DCHa & 0x07) * 4);
	writel(portb, pb_base + PB_CONFIG0 + (DCHa > 7 )*4 + (DCHa > 15) * 4);	
		
	portb = (readl(pb_base + PB_CONFIG0 + (PCHy2 > 7 )*4 + (PCHy2 > 15) * 4) & ~(0x0f << (PCHy2 & 0x07) * 4)) | (1 << (PCHy2 & 0x07) * 4);
	writel(portb, pb_base + PB_CONFIG0 + (PCHy2 > 7 )*4 + (PCHy2 > 15) * 4);
	portb = (readl(pb_base + PB_CONFIG0 + (DCHy2 > 7 )*4 + (DCHy2 > 15) * 4) & ~(0x0f << (DCHy2 & 0x07) * 4)) | (1 << (DCHy2 & 0x07) * 4);
	writel(portb, pb_base + PB_CONFIG0 + (DCHy2 > 7 )*4 + (DCHy2 > 15) * 4);	
	
	portb = (readl(pb_base + PB_CONFIG0 + (PCHt > 7 )*4 + (PCHt > 15) * 4) & ~(0x0f << (PCHt & 0x07) * 4)) | (1 << (PCHt & 0x07) * 4);
	writel(portb, pb_base + PB_CONFIG0 + (PCHt > 7 )*4 + (PCHt > 15) * 4);
	portb = (readl(pb_base + PB_CONFIG0 + (DCHt > 7 )*4 + (DCHt > 15) * 4) & ~(0x0f << (DCHt & 0x07) * 4)) | (1 << (DCHt & 0x07) * 4);
	writel(portb, pb_base + PB_CONFIG0 + (DCHt > 7 )*4 + (DCHt > 15) * 4);	
		
		
