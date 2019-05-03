#!/usr/bin/env python

""" Copyright (c) 2017-2018 Silex Inside. All Rights reserved """

import binascii
import os
import struct

##############################################################################
# Error handling
##############################################################################

class RunError(Exception):
   pass

def run(main):
   try:
      main()
   except RunError as e:
      print "ERROR: %s" % e
      exit(1)

##############################################################################
# Crypto packages
##############################################################################

help = """\
This script requires {0} package ({1}).

It can be installed with the following command:
> pip install {0}

If you don't have administrator right, it can be installed with:
> pip install --user {0}

On Windows, you might require installation of Microsoft Visual C++ Compiler
for Python 2.7 (http://aka.ms/vcpython27).
It might be needed to get a stdint.h file for Visual Studio and to copy it
to <Visual Studio install dir>/VC/include.
(https://www.google.be/search?q=stdint.h+visual+studio)
"""
import warnings
warnings.filterwarnings("ignore")
try:
   from Crypto.Cipher import AES
   from Crypto import Random
   from Crypto.Util import Counter
   from Crypto.Util.number import long_to_bytes, bytes_to_long
except ImportError:
   raise RunError(help.format("pycrypto", "https://www.dlitz.net/software/pycrypto/"))

try:
   import ed25519
except ImportError:
   raise RunError(help.format("ed25519", "https://github.com/warner/python-ed25519"))

try:
   import ecdsa
except ImportError:
   raise RunError(help.format("ecdsa", "https://github.com/warner/python-ecdsa/"))

import hashlib
import aes_gcm


##############################################################################
# Constants
##############################################################################

ENCR_KEYSIZE = 16

def get_privkey_size(algo):
   assert algo in ["ed25519", "ecdsa_p256", "ecdsa_p384"], "invalid algorithm"
   if algo == "ed25519" or algo == "ecdsa_p256":
      return 32
   if algo == "ecdsa_p384":
      return 48

def get_pubkey_size(algo):
   assert algo in ["ed25519", "ecdsa_p256", "ecdsa_p384"], "invalid algorithm"
   if algo == "ed25519":
      return 32
   if algo == "ecdsa_p256":
      return 64
   if algo == "ecdsa_p384":
      return 96

def get_sign_size(algo):
   assert algo in ["ed25519", "ecdsa_p256", "ecdsa_p384"], "invalid algorithm"
   if algo == "ed25519" or algo == "ecdsa_p256":
      return 64
   if algo == "ecdsa_p384":
      return 96

def get_cert_size(algo):
   return Certificate(algo).get_size()

def get_chipcert_size(algo):
   return Chip_certificate(algo).get_size()

def get_otpcm_size(algo, frk_size):
   return OTPCM(algo, frk_size).get_size()

def get_otpsm_size(algo, frk_size):
   return OTPSM(algo, frk_size).get_size()

##############################################################################
# File access
##############################################################################

def read(filename, nbytes=None):
   if filename==None:
      return ""
   print "Reading %s" % filename
   try:
      txt = open(filename, "rb").read()
   except:
      raise RunError("Cannot open file '%s' for read" % filename)
   if nbytes and len(txt)!=nbytes:
      raise RunError("File '%s' has invalid length" % filename)
   return txt

def write(filename, content, overwrite=True, tohex=False):
   if not filename:
      return
   print "Writing %s" % filename
   # Overwrite
   if not overwrite and os.path.exists(filename):
      raise RunError("File '%s' already exists" % filename)
   # Open
   try:
      f = open(filename, "wb")
   except:
      raise RunError("Cannot open file '%s' for write" % filename)
   # Write
   if tohex:
      assert len(content)%4==0
      for pos in range(0, len(content), 4):
         f.write("%08X\n" % struct.unpack("<I", content[pos:pos+4]))
   else:
      f.write(content)


##############################################################################
# Signature
##############################################################################

def create_keypair(algo, seed=None):

   assert algo in ["ed25519", "ecdsa_p256", "ecdsa_p384"], "invalid algorithm"

   if algo == "ed25519":
      if seed:
         signing_key = ed25519.SigningKey(seed, encoding="hex")
         verifying_key = signing_key.get_verifying_key()
      else:
         signing_key, verifying_key = ed25519.create_keypair()
      return signing_key.to_seed(), verifying_key.to_bytes()

   if "ecdsa" in algo:
      if algo == "ecdsa_p256":
         curve=ecdsa.NIST256p
      elif algo == "ecdsa_p384":
         curve=ecdsa.NIST384p

      if seed:
         seedbytes = binascii.unhexlify(seed)
         signing_key = ecdsa.SigningKey.from_string(seedbytes, curve=curve)
      else:
         signing_key = ecdsa.SigningKey.generate(curve=curve)
      verifying_key = signing_key.get_verifying_key()
      return signing_key.to_string(), verifying_key.to_string()

def sign(algo, key, message):

   assert algo in ["ed25519", "ecdsa_p256", "ecdsa_p384"], "invalid algorithm"
   if algo == "ed25519":
      signing_key = ed25519.SigningKey(key)
   if algo == "ecdsa_p256":
      signing_key = ecdsa.SigningKey.from_string(key, curve=ecdsa.NIST256p,
                                                      hashfunc=hashlib.sha256)
   if algo == "ecdsa_p384":
      signing_key = ecdsa.SigningKey.from_string(key, curve=ecdsa.NIST384p,
                                                      hashfunc=hashlib.sha384)

   signature = signing_key.sign(message)
   return signature

def verify(algo, key, message, signature):

   assert algo in ["ed25519", "ecdsa_p256", "ecdsa_p384"], "invalid algorithm"
   if algo == "ed25519":
      verifying_key = ed25519.VerifyingKey(key)
   if algo == "ecdsa_p256":
      verifying_key = ecdsa.VerifyingKey.from_string(key, curve=ecdsa.NIST256p,
                                                          hashfunc=hashlib.sha256)
   if algo == "ecdsa_p384":
      verifying_key = ecdsa.VerifyingKey.from_string(key, curve=ecdsa.NIST384p,
                                                          hashfunc=hashlib.sha384)

   try:
      verifying_key.verify(signature, message)
   except (ecdsa.BadSignatureError, ed25519.BadSignatureError) as e:
      return False
   else:
      return True


##############################################################################
# Encryption
##############################################################################

def create_random_key(length,seed=None):
   if seed:
      key = binascii.unhexlify(seed)
   else:
      key = os.urandom(length)
   return key

def get_KEK(firmware_root_key, certificate):
   kek_size = len(firmware_root_key)
   return hashlib.sha256(certificate + firmware_root_key).digest()[0:kek_size]

def get_aesctr_cipher(key):
   ctr = Counter.new(128, initial_value=0)
   cipher = AES.new(key, AES.MODE_CTR, counter=ctr)
   return cipher

def get_aescbc_cipher(key):
   cipher = AES.new(key, AES.MODE_CBC, IV=chr(0)*16)
   return cipher

def encrypt(KEK, plaintext):
   key_size = len(KEK)
   # Generate random software encryption key
   encryption_key = create_random_key(key_size)
   # Encrypt firmware
   cipher = get_aesctr_cipher(encryption_key)
   ciphertext = cipher.encrypt(plaintext)
   # Encrypt encryption key
   cipher = get_aescbc_cipher(KEK)
   encrypted_key = cipher.encrypt(encryption_key)
   return ciphertext, encrypted_key

def decrypt(KEK, encrypted_key, ciphertext):
   # Decrypt key
   cipher = get_aescbc_cipher(KEK)
   encryption_key = cipher.decrypt(encrypted_key)
   # Decrypt firmware
   cipher = get_aesctr_cipher(encryption_key)
   plaintext = cipher.decrypt(ciphertext)
   return plaintext

##############################################################################
# Structure
##############################################################################

class Structure(object):
   def __init__(self, algo=None):
      self.algo = algo
      self.content = 0
      self.nextpos = 0
      self.fields = {}
      self.listfields = []

   def add_field(self, name, size, bigend):
      if size%8==0:
         assert self.nextpos%8==0
      if name:
         self.fields[name] = (self.nextpos, size, bigend)
         self.listfields.append(name)
      self.nextpos += size

   def get_pos(self, key):
      try:
         pos, nbits, bigend = self.fields[key]
      except:
         raise RunError("invalid certificate field '%s'" % key)
      lsb = pos
      msb = lsb+nbits
      mask = 2**msb-2**lsb
      return nbits, mask, lsb, bigend

   def set_bits(self, key, value):
      nbits, mask, shift, bigend = self.get_pos(key)
      # Read from file
      if value.startswith('<'):
         value = "0x"+binascii.hexlify(read(value[1:-1], nbits/8))
      # Convert to integer
      try:
         value = int(value, 0)
      except:
         raise RunError("invalid %s value: '%s'" % (key, value))
      # Check range
      if value<0 or value>=2**nbits:
         raise RunError("%s value out of range: %#x" % (key, value))
      # Swap bytes
      if bigend:
         assert nbits%8==0
         value = "{0:0{1}x}".format(value, nbits/4)   # To hexadecimal string
         value = binascii.unhexlify(value)            # To bytes
         value = value[::-1]                          # Swap bytes
         value = binascii.hexlify(value)              # To hexadecimal string
         value = int(value, 16)                       # To integer
      self.content &= ~mask
      self.content |= value<<shift

   def get_bits(self, key):
      # Extract value
      nbits, mask, shift, bigend = self.get_pos(key)
      value = (self.content&mask)>>shift
      # Convert to hexadecimal string
      value = "{0:0{1}x}".format(value, nbits/4)
      # Swap bytes
      if bigend:
         assert nbits%8==0
         value = binascii.unhexlify(value)            # To bytes
         value = value[::-1]                          # Swap bytes
         value = binascii.hexlify(value)              # To hexadecimal string
      return value

   def set_txt(self, txt):
      for l in txt.splitlines():
         try:
            key, value = tuple(l.strip().split(None, 1))
         except:
            raise RunError("invalid format (%s)" % l)
         self.set_bits(key, value)

   def set_bin(self, bin):
      self.set_bits("all", "0x"+binascii.hexlify(bin))

   def get_size(self):
      return (self.nextpos+7)/8

   def get_txt(self):
      txt = ""
      for key in self.listfields:
         txt += "  %s 0x%s\n" % (key, self.get_bits(key))
      return txt.rstrip()

   def get_bin(self):
      return binascii.unhexlify(self.get_bits("all"))

##############################################################################
# Certificate
##############################################################################
class Certificate(Structure):
   def __init__(self, algo):
      super(Certificate, self).__init__(algo)         #nbits, bigend
      self.add_field("magic_number",                      32, False)
      self.add_field("dbg_grant",                         16, False)
      self.add_field("esecure_firmware",                   1, False)
      self.add_field("host_firmware",                      1, False)
      self.add_field("revoke_key",                         1, False)
      self.add_field(None,                                 5, False)
      self.add_field("key_index",                          8, False)
      self.add_field("tamper",                            32, False)
      self.add_field("serial",                           128, True)
      self.add_field("mask",                             128, True)
      self.add_field("pk",      8*get_pubkey_size(self.algo), True)
      self.add_field("signature", 8*get_sign_size(self.algo), True)
      # Whole certificate
      self.fields["all"]         = (0, self.nextpos, True)
      # Whole certificate without signature
      self.fields["nosignature"] = (0, self.nextpos-8*get_sign_size(self.algo), True)
      self.set_bits("magic_number", "0xE5ECCE01")

   def sign(self, key):
      signature = sign(self.algo, key, binascii.unhexlify(self.get_bits("nosignature")))
      self.set_bits("signature", "0x"+binascii.hexlify(signature))

   def verify(self, key):
      return verify(self.algo, key, binascii.unhexlify(self.get_bits("nosignature")), binascii.unhexlify(self.get_bits("signature")))

def sign_certificate(algo, txt_certificate, signing_key):
   cert = Certificate(algo)
   cert.set_txt(txt_certificate)
   cert.sign(signing_key)
   return cert.get_bin()

def verify_certificate(algo, bin_certificate, verifying_key):
   cert = Certificate(algo)
   cert.set_bin(bin_certificate)
   return cert.verify(verifying_key)

def decode_certificate(algo, bin_certificate):
   cert = Certificate(algo)
   cert.set_bin(bin_certificate)
   return cert.get_txt()

##############################################################################
# Firmware
##############################################################################

def gen_firmware(algo, bin, version, KEK, rootkeys_size, frk_select, signing_key, certificate):
   # Encrypt firmware
   if KEK:
      bin, encrypted_key = encrypt(KEK, bin)
      encrypt_flag = True
   else:
      encrypted_key = chr(0)*rootkeys_size
      encrypt_flag = False
   # Sign firmware
   to_be_signed = struct.pack("<2I", len(bin), version) + struct.pack("<2B",encrypt_flag,frk_select) + chr(0)*2 + encrypted_key + bin
   if signing_key:
      signature = sign(algo, signing_key, to_be_signed)
   else:
      certificate = chr(0) * get_cert_size(algo)
      signature = chr(0) * get_sign_size(algo)
   # Pack firmware
   return certificate + signature + to_be_signed

##############################################################################
# Flash
##############################################################################

def pad(bin, size):
   assert size-len(bin) >= 0
   return bin + chr(255) * (size-len(bin))

def gen_flash(empty,size,esec_fw_addrA,esec_fw_addrB,host_fw_addrA,host_fw_addrB,esec_fw_A,esec_fw_B,host_fw_A,host_fw_B):
   #Note: this piece of code assumes the order in flash is esec_fw_A, esec_fw_B, host_fw_A, host_fw_B
   if not empty:
      flash = pad(struct.pack("<4I",esec_fw_addrA,esec_fw_addrB,host_fw_addrA,host_fw_addrB),esec_fw_addrA)
      flash += pad(esec_fw_A,esec_fw_addrB-esec_fw_addrA) + pad(esec_fw_B,host_fw_addrA-esec_fw_addrB)
      flash += pad(host_fw_A,host_fw_addrB-host_fw_addrA) + pad(host_fw_B,size-host_fw_addrB)
   else:
      flash = pad("",size)
   return flash


##############################################################################
# Chip certificate
##############################################################################

class Chip_certificate(Structure):
   def __init__(self, algo):
      super(Chip_certificate, self).__init__(algo)    #nbits, bigend
      self.add_field("magic_number",                      32, False)
      self.add_field("serialno",                         128, True)
      self.add_field("pubEK",   8*get_pubkey_size(self.algo), True)
      self.add_field("signature", 8*get_sign_size(self.algo), True)
      # Whole certificate
      self.fields["all"]         = (0, self.nextpos, True)
      # Whole certificate without signature
      self.fields["nosignature"] = (0, self.nextpos-8*get_sign_size(self.algo), True)
      self.set_bits("magic_number", "0xE5ECCC01")

   def sign(self, key):
      signature = sign(self.algo, key, binascii.unhexlify(self.get_bits("nosignature")))
      self.set_bits("signature", "0x"+binascii.hexlify(signature))

   def verify(self, key):
      return verify(self.algo, key, binascii.unhexlify(self.get_bits("nosignature")), binascii.unhexlify(self.get_bits("signature")))

##############################################################################
# OTP
##############################################################################

class OTPCM(Structure):
   def __init__(self, algo, frk_size):
      super(OTPCM, self).__init__(algo)                #nbits, bigend
      self.add_field("HW_LOCK_BIT",                         1, False)
      self.add_field("ESEC_SECUREBOOT",                     1, False)
      self.add_field("WATCHDOG",                            1, False)
      self.add_field("ESEC_FW_ENABLE",                      1, False)
      self.add_field(None,                                  3, False)
      self.add_field("CHIP_TAMPERED",                       1, False)
      self.add_field("PUBCMREVOKED",                        8, False)
      self.add_field("DEBUGPROTENCM",                      16, False)
      self.add_field("SERIALNO",                          128, True)
      self.add_field("PUBCM0",  8*get_pubkey_size(self.algo), True)
      self.add_field("PUBCM1",  8*get_pubkey_size(self.algo), True)
      self.add_field("FRKCM0", frk_size * 8, True)
      self.add_field("FRKCM1", frk_size * 8, True)
      for i in range(32):
         self.add_field("TAMP%02d_CM"%i,                   4, False)
      self.add_field("TAMPERFILTERPERIODCM",               8, False)
      self.add_field("TAMPERFILTERTHRESHOLDCM",            8, False)
      self.add_field(None,                                16, False)
      self.add_field("CHIPCERT",                      8*1412, True)
      self.add_field("PRIVEK", 8*get_privkey_size(self.algo), True)
      self.fields["all"] = (0, self.nextpos, True)

class OTPSM(Structure):
   def __init__(self, algo, frk_size):
      super(OTPSM, self).__init__(algo)             #nbits, bigend
      self.add_field(None,                               1, False)
      self.add_field("HOST_SECUREBOOT",                  1, False)
      self.add_field(None,                               1, False)
      self.add_field("HOST_FW_ENABLE",                   1, False)
      self.add_field(None,                               4, False)
      self.add_field("PUBSMREVOKED",                     8, False)
      self.add_field("DEBUGPROTENSM",                    16, False)
      self.add_field("PUBSM0", 8*get_pubkey_size(self.algo), True)
      self.add_field("PUBSM1", 8*get_pubkey_size(self.algo), True)
      self.add_field("FRKSM0", frk_size * 8, True)
      self.add_field("FRKSM1", frk_size * 8, True)
      for i in range(32):
         self.add_field("TAMP%02d_SM"%i,                  4, False)
      self.add_field("TAMPERFILTERPERIODSM",              8, False)
      self.add_field("TAMPERFILTERTHRESHOLDSM",           8, False)
      self.add_field(None,                               16, False)
      self.fields["all"] = (0, self.nextpos, True)

def create_OTP_page(puf, otp):
   if puf == 'flex':
      srk = 0xB49836B823C385A179BC8EA77F2DA0C10802DA892DFF38B715A655D4DD894957
      keylen = 256
   else:
      srk = 0xC1A717F778C1B153A0061D6E4D643E70
      keylen = 128
   my_gcm = aes_gcm.AES_GCM(srk, keylen = keylen)
   iv = os.urandom(12)
   encr, tag = my_gcm.encrypt(bytes_to_long(iv), otp)
   return iv + encr + long_to_bytes(tag, 16)

def add_OTP_to_flash(puf, flash, addr, otpCM, otpSM):
   if puf == 'flex':
      activation_code = binascii.unhexlify('3221C56C4376C0447B5875A448B254B28C81226F3B702A717B2A3CB4418738CBC66FE2943B2CC31A955EB0A10AE86B942467BE09CE6C3E08E393A334A2CB594C1979458A864AA02B0DF25159E0292DE4818C6014ED09F3D543DB05EE576E30EA96394AEB7026E0968E4F08D06E3863D18599C37F9E262B7A9346858E385C063BECD61CB7278113AE772C7171E7E393B374EB53EC3E5615C62DAF825E8CB5791853221ABC69D4B315A459D2458C0F7D49103375170ED80F6D08E030538B5D3D015C8CA591F59244E3E6417326C032074911CF5D92ED0D78EEDCAB2DB7093FB13DEB42265A8AF690245478BDE41E2BC1AD1B34FD67BEFA85BE58C5094C87B9B48918A8EBBD698E40E3E8C1476F6FE987F62A505C72CA91D9597A6BDF45C42B9E5F1631594C8E601117D79AB22A8219D086942009352DA1C71437368C579844FD6D0AD7876E2FC81FB7DBB539B5596637F4316B50DC9B156ED11D2B9AE901BD3CB4C7F080B6A0815F60B6D43F9C76057777E641C9CD3EBA05BEB0618CA3AAED90B13E1211B312E637048642DE58EC0E278BF9B5A9CA40D063F9DFB42B2B51CEF6711152B4C91518CCEDA660CD1A4FC5B3DF7BE0FBF89AB48FB062B039F29B756AEE92467EC612C7BFE0A774FB56C3518C57FFA8BC1010A56AA04882E8B5B24A2730ADD3364FA98967D71816C7301D32E5E6E568C84CD596CE7BFCEC605999801CD42B8290A14B6911CA4B0AA0DE269D8BA0BA796577C7CD3CA863FA81B362D6C7569DA32F26E9EAF2E7EC7C14262E4EC1B852A3943051ABE812B154D7BF885B62B13B9CEDF8C55E4E56CDFC3FEA1968E26866F123BB6131B7547EFE33E3907967FB65632BF8B199063BD6E073C4043100D80264A4A968EDD8150CB4E0AF3D0A957E16E406ED6337099459A46E406263EA5C8B111F7F906638D5D637FE4EE8C9F777A123FB13A25501F148E28CF8559A03F0FC03D514D53D62599096593E1F8B51C86F0C4AA660C51AFF0587F2ACD8BF1D3984105753BAD47CF1FD6880217EAF6D316322231CD930B1B3B15868DCC6C595C5152006B765CCDEA150173B4BA36798D9A9751A1B7C0C9BAA25F0D2858E1F83B679A18F29B4E3A728183B973D893878E6D8B2B750A637598C90CC5F69EDEB4460439D64ED7E493301284422CB267B3D756583EE41745FCB505A78599140FD05D82D331FB3D75C52EABED8C9697ACFF85B86E73DAC82CBDD42CE20142ABA0FB19CA9B40CDF87707C01296B0B61679D190FCD334DCE448E5928F273188F0B1221E53B5B15C017C9C35FE4EEFB893E429A748D3B48348B7C1A732D07C31B8ADC1387F54AFF6853CF1FD0A229C33AD0E7A513CE4DC802D4545C503A5670EC309FCD76A46AF4F4F5D7C8CB659555C257C1386F56D3F8AA4E8CCFDA215DD885A4EB4EE0F9A2A1EA89D2A276E37953C3198C9F93970453D5FFAC02AF3A9F64C925EC2D7C51B016A96B15250653B728279541CF64DF2AF3DCF16EDE8A719EA5C6945453C6B5BD82C74FB1E6F87FAF774C4D7DCC2583298EC054CC064C10646F6F8A2987B048D3C395E2B6C7F5B9E187CFC8EA4EE009E20A53ED0A2059D1D3CD57A4665BE5CA6DEE0A515AC6EE6608C3354AE753C6A28419C51DD815E91CB6868F1D3621C52023BED12D02FFA59F7038C4C24BE3210A091F19C09316B9')
      activation_code += '0' * 32 # "random" nonce 32 bytes 0x30
   else:
      activation_code = binascii.unhexlify('E069C33B56434168DE896E7D7C5BDB17DF021F6AB029F6B93C91B70611F2C69294E764C4EAB9726ECC77C456A731634B705D1EEE20DD8EF0B463C8D7C0B249B7A7CE85A9D878A0D5853250E942E0683237C7D64ED15740BAC3069E9E4B393F7CD38E66FF208F778B23484DE7C359434F7EFEC8F79D3FF0AEC8B5F318EC8FC859C1595559BB8D6BDDEA12EA979ED497B32EA2A7A4606CBB79CFC8F75CCC680439442B731B4481647A5440226FF1EB178C84572587D10B60EBAB41AD5E9CDD0773907AB2177C861ED1386059D3D557A449DE39BCA835D63EA6C1302F3695347229DB438216B480ACDF4D30918ADE7D24533B94F54B7AF82FEDF12C4E8B9E75EC09D3876C9F2DD3918CE743A1108AF28E0B61E8012E04C516993BB8E86262C829E6D6F02B3BDD94392B52F2572EDF0BD50B3FF8A8FBD3D668206FFEFA29F05543F3EA7DA013A8C0EAEC83312BF026D0EC0DF9FC2A7AED39515123CC0A8FFDF0696769CE477BAAD5D632A47F178616B6A97D003171B588605149EAA8F6D39A48E1EBC5032A4E3A83FF5B17E5DFC2B820A407408671A49F5D6B4C30AC8548A69FABC66C0F3AF34A45AE2CED17E11959159DED3C33813799125FE2079B44226B58941DB5C8DAAD33F8623A68C6B51A95A9A1D4077880037E02777919431207789AC01C9710DD372B1C516207B47FF0FE9606B78B79BE5B9E302FBA1F3292E1FC3F5BF3F2134489EBB48824CE1C7E0114CC0EC7AEBCB5DCA627DC15E95FD23892A1EC3D2A1BBE847E1FFED05CB0CE1D485B284CF14F9ECE56D19DA3061D10570AF59D0E2D8DEA9C09B943070EDCFEBB745CE4F0F86B7287FF643DF4639C9D770E38D7D65A048ED68B965B19AB818CC0125DBAA99DE3BD86670249AD7D82B592ADF801E65E45A9F5F64B37070047C1E88CAD3B789F5DFA4F9EEA4AA2FB37B8C666140CBED421CF98F17749AB088010D56535DDBB0B6110FBDB56539EE86710616A15453ED31EA2EA10D9D8718EC0BED9E9267E81D8971967634F7683124C1BB8516048A91CDEFAE583F737068424CDE6264055238B8151D57B995324C20DE3071EF7B19128900780674C460C1E4802F16D12B4DB4D1E6B28F3643D2855A6938D017748A6D8436809CB355E85B20678C4906796F0F4C7F33F59576E79EA61B80F0EEEBCE6C72BB3EBAE7AEC3741DE5F98794507C723786E79E0024F871CDB64D8AB7D873A929C307915F7F874BEBF240734F8DC1CD38F1B21FDD07D56')
   otpCM_page = create_OTP_page(puf, otpCM)
   otpSM_page = create_OTP_page(puf, otpSM)
   OTP_section = activation_code + otpCM_page + otpSM_page
   flash = flash[0:addr] + pad(OTP_section, len(flash)-addr)
   return flash, OTP_section

##############################################################################
# Autenticated commands
##############################################################################

def sign_command(algo, key, cmd, param, challenge = None):
   msg = binascii.unhexlify(cmd)[::-1] + binascii.unhexlify(param)[::-1]
   if challenge:
      msg += binascii.unhexlify(challenge)
   print 'msg = ', binascii.hexlify(msg)
   return sign(algo, key, msg)

##############################################################################
# ROM
##############################################################################

##############################################################################
# Self-test
##############################################################################

if __name__=="__main__":
   message = "Tripel Karmeliet"

   for algo in ["ed25519", "ecdsa_p256", "ecdsa_p384"]:

      sk, vk = create_keypair(algo)
      signature = sign(algo, sk, message)
      assert verify(algo, vk, message, signature)
      assert not verify(algo, vk, message+"x", signature)

      cert = Certificate(algo)
      cert.set_bits("tamper", "0x12345678")
      assert cert.get_bits("tamper")=="12345678"
      cert.sign(sk)
      assert cert.verify(vk)
      cert.set_bits("tamper", "0x12345679")
      assert not cert.verify(vk)

      frk_size = 16
      frk = create_random_key(frk_size)
      KEK = get_KEK(frk, cert.get_bin())
      ciphertext, encrypted_key = encrypt(KEK, message)
      assert decrypt(KEK, encrypted_key, ciphertext) == message


