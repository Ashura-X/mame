// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_CPU_M6805_M68HC05_H
#define MAME_CPU_M6805_M68HC05_H

#pragma once

#include "m6805.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

extern device_type const M68HC05C4;
extern device_type const M68HC05C8;
extern device_type const M68HC705C8A;


//**************************************************************************
//  TYPE DECLARATIONS
//**************************************************************************

// ======================> m68hc05_device

#define MCFG_M68HC05_PORTA_R_CB(obj) \
	devcb = &m68hc05_device::set_port_cb_r<0>(*device, DEVCB_##obj);

#define MCFG_M68HC05_PORTB_R_CB(obj) \
	devcb = &m68hc05_device::set_port_cb_r<1>(*device, DEVCB_##obj);

#define MCFG_M68HC05_PORTC_R_CB(obj) \
	devcb = &m68hc05_device::set_port_cb_r<2>(*device, DEVCB_##obj);

#define MCFG_M68HC05_PORTD_R_CB(obj) \
	devcb = &m68hc05_device::set_port_cb_r<3>(*device, DEVCB_##obj);

#define MCFG_M68HC05_PORTA_W_CB(obj) \
	devcb = &m68hc05_device::set_port_cb_w<0>(*device, DEVCB_##obj);

#define MCFG_M68HC05_PORTB_W_CB(obj) \
	devcb = &m68hc05_device::set_port_cb_w<1>(*device, DEVCB_##obj);

#define MCFG_M68HC05_PORTC_W_CB(obj) \
	devcb = &m68hc05_device::set_port_cb_w<2>(*device, DEVCB_##obj);

#define MCFG_M68HC05_PORTD_W_CB(obj) \
	devcb = &m68hc05_device::set_port_cb_w<3>(*device, DEVCB_##obj);

#define MCFG_M68HC05_TCMP_CB(obj) \
	devcb = &m68hc05_device::set_tcmp_cb(*device, DEVCB_##obj);


class m68hc05_device : public m6805_base_device
{
public:
	// static configuration helpers
	template<std::size_t N, typename Object> static devcb_base &set_port_cb_r(device_t &device, Object &&obj)
	{ return downcast<m68hc05_device &>(device).m_port_cb_r[N].set_callback(std::forward<Object>(obj)); }
	template<std::size_t N, typename Object> static devcb_base &set_port_cb_w(device_t &device, Object &&obj)
	{ return downcast<m68hc05_device &>(device).m_port_cb_w[N].set_callback(std::forward<Object>(obj)); }
	template<typename Object> static devcb_base &set_tcmp_cb(device_t &device, Object &&obj)
	{ return downcast<m68hc05_device &>(device).m_tcmp_cb.set_callback(std::forward<Object>(obj)); }

protected:
	// state index constants
	enum
	{
		M68HC05_A = M6805_A,
		M68HC05_PC = M6805_PC,
		M68HC05_S = M6805_S,
		M68HC05_X = M6805_X,
		M68HC05_CC = M6805_CC,
		M68HC05_IRQ_STATE = M6805_IRQ_STATE,

		M68HC05_TCR = 0x10,
		M68HC05_TSR,
		M68HC05_ICR,
		M68HC05_OCR,
		M68HC05_PS,
		M68HC05_TR,

		M68HC05_COPRST,
		M68HC05_COPCR,
		M68HC05_PCOP,
		M68HC05_NCOPE,
		M68HC05_NCOP
	};

	enum { PORT_COUNT = 4 };

	m68hc05_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			u32 clock,
			device_type type,
			char const *name,
			address_map_delegate internal_map,
			char const *shortname,
			char const *source);

	void set_port_bits(u8 a, u8 b, u8 c, u8 d);
	DECLARE_READ8_MEMBER(port_r);
	DECLARE_WRITE8_MEMBER(port_latch_w);
	DECLARE_READ8_MEMBER(port_ddr_r);
	DECLARE_WRITE8_MEMBER(port_ddr_w);

	DECLARE_READ8_MEMBER(tcr_r);
	DECLARE_WRITE8_MEMBER(tcr_w);
	DECLARE_READ8_MEMBER(tsr_r);
	DECLARE_READ8_MEMBER(icr_r);
	DECLARE_READ8_MEMBER(ocr_r);
	DECLARE_WRITE8_MEMBER(ocr_w);
	DECLARE_READ8_MEMBER(timer_r);

	void set_ncope(bool state) { m_ncope = state ? 1 : 0; }
	DECLARE_WRITE8_MEMBER(coprst_w);
	DECLARE_READ8_MEMBER(copcr_r);
	DECLARE_WRITE8_MEMBER(copcr_w);
	DECLARE_WRITE8_MEMBER(copr_w);

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void execute_set_input(int inputnum, int state) override;
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const override;
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const override;

	virtual offs_t disasm_disassemble(
			std::ostream &stream,
			offs_t pc,
			const uint8_t *oprom,
			const uint8_t *opram,
			uint32_t options) override;

	virtual void interrupt() override;
	virtual void burn_cycles(unsigned count) override;

	void add_timer_state();
	void add_pcop_state();
	void add_ncop_state();

private:
	u8 port_value(unsigned offset) const
	{
		return (m_port_latch[offset] & m_port_ddr[offset]) | (m_port_input[offset] & ~m_port_ddr[offset]);
	}

	bool    tcr_icie() const    { return BIT(m_tcr, 7); }
	bool    tcr_ocie() const    { return BIT(m_tcr, 6); }
	bool    tcr_toie() const    { return BIT(m_tcr, 5); }
	bool    tcr_iedg() const    { return BIT(m_tcr, 1); }
	bool    tcr_olvl() const    { return BIT(m_tcr, 0); }

	bool    tsr_icf() const     { return BIT(m_tsr, 7); }
	bool    tsr_ocf() const     { return BIT(m_tsr, 6); }
	bool    tsr_tof() const     { return BIT(m_tsr, 5); }

	bool    copcr_copf() const  { return BIT(m_copcr, 4); }
	bool    copcr_cme() const   { return BIT(m_copcr, 3); }
	bool    copcr_pcope() const { return BIT(m_copcr, 2); }
	u8      copcr_cm() const    { return m_copcr & 0x03; }

	// digital I/O
	devcb_read8         m_port_cb_r[PORT_COUNT];
	devcb_write8        m_port_cb_w[PORT_COUNT];
	u8                  m_port_bits[PORT_COUNT];
	u8                  m_port_input[PORT_COUNT];
	u8                  m_port_latch[PORT_COUNT];
	u8                  m_port_ddr[PORT_COUNT];

	// timer/counter
	devcb_write_line    m_tcmp_cb;
	bool                m_tcap_state;
	u8                  m_tcr;
	u8                  m_tsr, m_tsr_seen;
	u8                  m_prescaler;
	u16                 m_counter, m_icr, m_ocr;
	bool                m_inhibit_cap, m_inhibit_cmp;
	u8                  m_trl_buf[2];
	bool                m_trl_latched[2];

	// COP watchdogs
	u32                 m_pcop_cnt, m_ncop_cnt;
	u8                  m_coprst, m_copcr;
	u8                  m_ncope;
};


// ======================> m68hc705_device

class m68hc705_device : public m68hc05_device
{
protected:
	m68hc705_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			u32 clock,
			device_type type,
			char const *name,
			address_map_delegate internal_map,
			char const *shortname,
			char const *source);
};


// ======================> m68hc05c4_device

class m68hc05c4_device : public m68hc05_device
{
public:
	m68hc05c4_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	DECLARE_ADDRESS_MAP(c4_map, 8);

	virtual void device_start() override;

	virtual offs_t disasm_disassemble(
			std::ostream &stream,
			offs_t pc,
			const uint8_t *oprom,
			const uint8_t *opram,
			uint32_t options) override;
};


// ======================> m68hc05c8_device

class m68hc05c8_device : public m68hc05_device
{
public:
	m68hc05c8_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	DECLARE_ADDRESS_MAP(c8_map, 8);

	virtual void device_start() override;

	virtual offs_t disasm_disassemble(
			std::ostream &stream,
			offs_t pc,
			const uint8_t *oprom,
			const uint8_t *opram,
			uint32_t options) override;
};


// ======================> m68hc705c8a_device

class m68hc705c8a_device : public m68hc705_device
{
public:
	m68hc705c8a_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	DECLARE_ADDRESS_MAP(c8a_map, 8);

	virtual tiny_rom_entry const *device_rom_region() const override;

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual offs_t disasm_disassemble(
			std::ostream &stream,
			offs_t pc,
			const uint8_t *oprom,
			const uint8_t *opram,
			uint32_t options) override;
};


/****************************************************************************
 * 68HC05 section
 ****************************************************************************/

#define M68HC05_INT_MASK            0x03
#define M68HC05_IRQ_LINE            (M6805_IRQ_LINE + 0)
#define M68HC05_TCAP_LINE           (M6805_IRQ_LINE + 1)

#endif // MAME_CPU_M6805_M68HC05_H