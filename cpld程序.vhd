Library ieee;
Use ieee.std_logic_1164.all;
Use ieee.std_logic_unsigned.all;
Use ieee.std_logic_arith.all;

Entity iis is
  generic(--M: integer:=4;	--bck rate,4 means /4
			n: integer:=64  --lrck rate,64 means /64
  );

  port(
        rstin: IN std_logic;--0:stop,1:working
        clkin: IN std_logic;--45.1584mhz
        clks0: IN std_logic;
        clks1: IN std_logic;
        bckout: buffer std_logic;
        lrckout: buffer std_logic;
        bckout2: OUT std_logic;
        lrckout2: OUT std_logic;
        rstout: OUT std_logic;
        rstoutn: OUT std_logic;
        passi: IN std_logic;
        passo: OUT std_logic
        );
End iis;

Architecture a of iis is
--  signal cnt: integer range 0 to M-1;
  signal cntl: integer range 0 to n-1;
  
  signal clkdiva:std_logic;--22.5792mhz
  signal clkdivb:std_logic;--11.2896mhz
  signal clkdivc:std_logic;--5.6448mhz
  signal mclk128fs: std_logic;----divided mclk

Begin

------1st  /2 divider ,in=45 out=22
process(clkin,rstin) 
begin
	if(rstin='0') then --异步复位
		clkdiva<='0';
	elsif(clkin'event and clkin='1') then ---触发器2分频
		clkdiva<=not clkdiva;
	end if;
end process;

------2nd  /2 divider,in=22,out=11
process(clkdiva,rstin) 
begin
	if(rstin='0') then --异步复位
		clkdivb<='0';
	elsif(clkdiva'event and clkdiva='1') then ---触发器2分频
		clkdivb<=not clkdivb;
	end if;
end process;
------3rd  /2 divider,in=11 out=5
process(clkdivb,rstin) 
begin
	if(rstin='0') then --异步复位
		clkdivc<='0';
	elsif(clkdivb'event and clkdivb='1') then ---触发器2分频
		clkdivc<=not clkdivc;
	end if;
end process;

----时钟选择器 clock selecter
process(clks0,clks1,clkin,clkdiva,clkdivb,clkdivc)
begin
	if (clks0='0' and clks1='0')then
		mclk128fs<=clkin;
	elsif (clks0='0' and clks1='1')then
		mclk128fs<=clkdiva;
	elsif (clks0='1' and clks1='0')then
		mclk128fs<=clkdivb;
	else  ---(clks0='1' and clks1='1')
		mclk128fs<=clkdivc;
	end if;
end process;


----mclk=128fs ,bck=64fs ,so ,mclk/2=bck
process(mclk128fs,rstin) 
begin
	if(rstin='0') then --异步复位
		bckout<='0';---bck start at logic 0
	elsif(mclk128fs'event and mclk128fs='1') then ---触发器2分频
		bckout<=not bckout;
	end if;
end process;


---64bit per frame
--- bck /64 =lrck
process(bckout,rstin) 
begin
	if (rstin='0')then --异步复位
			cntl <= 0;
  
	elsif(bckout'event and bckout='1') then

		if(cntl<n-1) then
			cntl <= cntl+1;
		else
			cntl <= 0;
		end if;
	end if;
end process;

process(cntl,rstin)  
begin
	if (rstin='0')then
		lrckout <= '1';  --lrck start at left channel,logic 1
	elsif(cntl<n/2) then
		lrckout <= '1';
	else
		lrckout <= '0';
	end if;
end process;

process(bckout,lrckout)  
begin

		bckout2 <= bckout;
		lrckout2 <=  lrckout;

end process;

process(rstin)  
begin

		rstoutn <= not rstin;
		rstout <= rstin;

end process;

process(passi)  
begin

		passo <= passi;

end process;

End a;