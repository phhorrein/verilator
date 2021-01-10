/*******************************************************************************
* vpi_compatibility.h
*
* IEEE 1800-2012 SystemVerilog: Verilog Procedural Interface (PLI/VPI)
*
* NOTE: THIS FILE IS INCLUDED BY vpi_user.h. DO NOT INCLUDE THIS FILE FROM
* USER APPLICATION CODE.
*
* This file contains the macro definitions used by the SystemVerilog PLI
* to implement backwards compatibility mode functionality.
*
******************************************************************************/
/* $Id$ */

#ifdef VPI_COMPATIBILITY_H
#error "The vpi_compatibility.h file can only be included by vpi_user.h directly."
#endif
#define VPI_COMPATIBILITY_H
/* Compatibility-mode variants of functions */
#if VPI_COMPATIBILITY_VERSION_1364v1995
#warning "VPI_COMPATIBILITY_VERSION_1364v1995 is unsupported, possible undefined behavior"
#if VPI_COMPATIBILITY_VERSION_1364v2001 \
 || VPI_COMPATIBILITY_VERSION_1364v2005 \
 || VPI_COMPATIBILITY_VERSION_1800v2005 \
 || VPI_COMPATIBILITY_VERSION_1800v2009 \
 || VPI_COMPATIBILITY_VERSION_1800v2012
#error "Only one VPI_COMPATIBILITY_VERSION symbol definition is allowed."
#endif
#elif VPI_COMPATIBILITY_VERSION_1364v2001
#warning "VPI_COMPATIBILITY_VERSION_1364v2001 is unsupported, possible undefined behavior"
#if VPI_COMPATIBILITY_VERSION_1364v1995 \
 || VPI_COMPATIBILITY_VERSION_1364v2005 \
 || VPI_COMPATIBILITY_VERSION_1800v2005 \
 || VPI_COMPATIBILITY_VERSION_1800v2009 \
 || VPI_COMPATIBILITY_VERSION_1800v2012
#error "Only one VPI_COMPATIBILITY_VERSION symbol definition is allowed."
#endif
#elif VPI_COMPATIBILITY_VERSION_1364v2005
#warning "VPI_COMPATIBILITY_VERSION_1364v2005 is unsupported, possible undefined behavior"
#if VPI_COMPATIBILITY_VERSION_1364v1995 \
 || VPI_COMPATIBILITY_VERSION_1364v2001 \
 || VPI_COMPATIBILITY_VERSION_1800v2005 \
 || VPI_COMPATIBILITY_VERSION_1800v2009 \
 || VPI_COMPATIBILITY_VERSION_1800v2012
#error "Only one VPI_COMPATIBILITY_VERSION symbol definition is allowed."
#endif
#elif VPI_COMPATIBILITY_VERSION_1800v2005
#warning "VPI_COMPATIBILITY_VERSION_1800v2005 is unsupported, possible undefined behavior"
#if VPI_COMPATIBILITY_VERSION_1364v1995 \
 || VPI_COMPATIBILITY_VERSION_1364v2001 \
 || VPI_COMPATIBILITY_VERSION_1364v2005 \
 || VPI_COMPATIBILITY_VERSION_1800v2009 \
 || VPI_COMPATIBILITY_VERSION_1800v2012
#error "Only one VPI_COMPATIBILITY_VERSION symbol definition is allowed."
#endif
#elif VPI_COMPATIBILITY_VERSION_1800v2009
#warning "VPI_COMPATIBILITY_VERSION_1800v2009 is unsupported, possible undefined behavior"
#if VPI_COMPATIBILITY_VERSION_1364v1995 \
 || VPI_COMPATIBILITY_VERSION_1364v2001 \
 || VPI_COMPATIBILITY_VERSION_1364v2005 \
 || VPI_COMPATIBILITY_VERSION_1800v2005 \
 || VPI_COMPATIBILITY_VERSION_1800v2012
#error "Only one VPI_COMPATIBILITY_VERSION symbol definition is allowed."
#endif
#elif VPI_COMPATIBILITY_VERSION_1800v2012
#warning "VPI_COMPATIBILITY_VERSION_1800v2012 is unsupported, possible undefined behavior"
#if VPI_COMPATIBILITY_VERSION_1364v1995 || \
    VPI_COMPATIBILITY_VERSION_1364v2001 || \
    VPI_COMPATIBILITY_VERSION_1364v2005 || \
    VPI_COMPATIBILITY_VERSION_1800v2005 || \
    VPI_COMPATIBILITY_VERSION_1800v2009
#error "Only one VPI_COMPATIBILITY_VERSION symbol definition is allowed."
#endif
#endif
