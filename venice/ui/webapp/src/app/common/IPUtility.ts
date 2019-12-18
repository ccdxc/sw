import { Rule, PortRule, IPRule } from '@app/models/frontend/shared/rule.interface';
import { AbstractControl, ValidationErrors } from '@angular/forms';


export class IPUtility {

  public static extractPortFromString(input: string) {
    // We gather all the letters, and all the digits
    // and assume that these form the protocol and the port

    // We skip the '/' character
    let letters = '';
    let numbers = '';
    for (let i = 0; i < input.length; i++) {
      if (input[i] === ' ' || input[i] === '/') {
        continue;
      }
      const isNumber = !isNaN(parseInt(input[i], 10));
      if (isNumber) {
        numbers += input[i];
      } else {
        letters += input[i];
      }
    }
    let port = parseInt(numbers, 10);
    if (isNaN(port)) {
      port = null;
    }
    return { port: port, protocol: letters };
  }

  /**
   * Returns true if the ip is valid
   * Does not allow submasks
   */
  public static isValidIP(ip: string): boolean {
    if (ip == null || ip.length === 0 || ip.indexOf('/') >= 0) {
      return false;
    }
    // Can't begin or end in a dot
    if (ip[0] === '.' || ip[ip.length - 1] === '.') {
      return false;
    }
    const ipParts = ip.split('.');
    if (ipParts.length !== 4) {
      return false;
    }
    // None of the octets can be greater than 255
    if (ipParts.some(
      (part) => {
        // fix 1.1.1.1a is a valid ip issue
        if (isNaN(Number(part))) {
          return true;
        }
        const octet = parseInt(part, 10);
        return octet < 0 || octet > 255;
      }
    )) {
      return false;
    }
    return true;
  }

  // check whether input string is valid submask number
  public static isValidSubmask(mask: string): boolean {
    const number = parseInt(mask, 10);
    if (isNaN(number)) {
      return false;
    }
    return number > 1 && number < 32;
  }

  /**
   * Returns true if the ip with mask is valid
   * mask is optional
   */
  public static isValidIPWithOptionalMask(ip: string): boolean {
    const arr: string[] = ip.split('/');
    if (arr.length > 2) {
      return false;
    }
    if (!IPUtility.isValidIP(arr[0])) {
      return false;
    }
    if (arr.length === 1) {
      return true;
    }
    return IPUtility.isValidSubmask(arr[1]);
  }

  /**
   * Returns true if the ip with mask is valid
   * mask must be provided
   */
  public static isValidIPWithMask(ip: string): boolean {
    const arr: string[] = ip.split('/');
    if (arr.length !== 2) {
      return false;
    }
    if (!IPUtility.isValidIP(arr[0])) {
      return false;
    }
    return IPUtility.isValidSubmask(arr[1]);
  }

  public static isValidIPValidator(control: AbstractControl): ValidationErrors | null {
    if (control.value == null || control.value.length === 0) {
      return null;
    }
    return !IPUtility.isValidIP(control.value) ? { validIP: { message: 'Must be a valid IP in dot notation' } } : null;
  }

  public static isValidIPListValidator(control: AbstractControl): ValidationErrors | null {
    if (control.value == null || control.value.length === 0) {
      return null;
    }
    const invalidIP = control.value.find( (ip) => {
      return !IPUtility.isValidIP(ip);
    });

    return invalidIP != null ? { validIP: { message: invalidIP + ' is not a valid IP in dot notation' } } : null;
  }

  /**
   * Creates a fully formed IP from a partially given ip
   * by adding wildcards for the remaining octets
   */
  public static generateFullIP(ip) {
    if (ip == null || ip === '') {
      ip = '*';
    }
    if (ip.endsWith('.')) {
      ip = ip.substring(0, ip.length - 1);
    }

    const parts = ip.split('.');
    for (let i = 0; i < 4 - parts.length; i++) {
      ip += '.*';
    }
    return ip;
  }

  /**
   * Returns whether addr2 is a substring of addr1
   * Used for showing matches of IPs that haven't been fully finished
   * Ex. entry is 192.10.1.20 and user has only entered 192.1
   */
  public static partialMatchIPAddress(refAddr: string, searchAddr: string) {
    const searchIPOcts = searchAddr.split('.');
    const refIPOcts = refAddr.split('.');

    // Octets need to be exact if not last octet
    for (let i = 0; i < searchIPOcts.length - 1; i++) {
      if (searchIPOcts[i] === '*' || refIPOcts[i] === '*') {
        continue;
      }

      if (searchIPOcts[i] + '.' !== refIPOcts[i] + '.') {
        return false;
      }
    }

    const refOctString = refIPOcts[searchIPOcts.length - 1];
    const searchOctString = searchIPOcts[searchIPOcts.length - 1];
    if (refOctString === '*' || searchOctString === '*') {
      return true;
    }
    // Last searchAddr octet we check if its a beginning substring of refAddr
    return refOctString.startsWith(searchOctString);
  }

  /**
   * returns < 0 if addr1 < addr2
   *         0 if addr1 == addr2
   *         > 0 if addr1 > addr2
   * addr1 and addr2 can be of incomplete form
   * If there is a mask, it is expected in CIDR form
   *
   * NOTE: if an address has a wild card in it, that octet is treated as equal.
   */
  public static compareIPAddress(addr1: string, addr2: string): number {
    if (addr1 === '' || addr1 == null) {
      return -1;
    } else if (addr2 === '' || addr2 == null) {
      return 1;
    }

    // Convert to full form
    const addr1Full = this.generateFullIP(addr1);
    const addr2Full = this.generateFullIP(addr2);

    const addr1FullParts = addr1Full.split('.'),
      addr2FullParts = addr2Full.split('.');
    let mask = 32;
    if (addr1Full.indexOf('/') >= 0) {
      mask = Math.min(addr1Full.split('/')[1], mask);
    }
    if (addr2Full.indexOf('/') >= 0) {
      mask = Math.min(addr2Full.split('/')[1], mask);
    }

    const maskIP = this.cdirToNetmask(mask).split('.');

    for (let i = 0; i < addr1FullParts.length; i++) {
      // Handle wildcard
      if (addr1FullParts[i] === '*' || addr2FullParts[i] === '*') {
        continue;
      }

      // Disable linting to allow us to use bitwise operators for the mask
      /* tslint:disable:no-bitwise */
      const oct1 = parseInt(addr1FullParts[i], 10) & parseInt(maskIP[i], 10);
      const oct2 = parseInt(addr2FullParts[i], 10) & parseInt(maskIP[i], 10);
      /* tslint:enable:no-bitwise */
      if (oct1 < oct2) {
        return -1;
      } else if (oct1 > oct2) {
        return 1;
      }
    }
    return 0;
  }

  /* Checks the Rule's source and dest IP. */
  public static filterRuleByIPv4(rule: Rule, searchIP: string): boolean {
    if (searchIP === '' || searchIP == null) {
      return true;
    }
    // check if search has invalid characters or invalid formatting
    const charCheck = RegExp('^[0-9./]*$');
    if (!charCheck.test(searchIP)) {
      return false;
    }
    let isValid = true;
    searchIP.split('.').forEach((oct) => {
      if (oct.length > 3) {
        isValid = false;
      }
    });
    if (!isValid) {
      return false;
    }
    const ruleIPsrc = rule.sourceIP;
    const ruleIPtgt = rule.destIP;
    if (this.matchIPv4(ruleIPsrc, searchIP)) {
      return true;
    }
    if (this.matchIPv4(ruleIPtgt, searchIP)) {
      return true;
    }
    return false;
  }

  /**
   * Given a mask size, generates the corresponding IP
   */
  public static cdirToNetmask = (bitCount) => {
    if (bitCount < 0) {
      bitCount = 0;
    }
    const mask = [];
    for (let i = 0; i < 4; i++) {
      const n = Math.min(bitCount, 8);
      mask.push(256 - Math.pow(2, 8 - n));
      bitCount -= n;
    }
    return mask.join('.');
  }

  /* Checks an individual ip address, not a range */
  public static matchIPv4Helper(ipv4Address: any, searchIP: string): boolean {
    let ret = this.compareIPAddress(searchIP, ipv4Address.toString()) === 0;
    if (ret) {
      return true;
    }
    ret = this.partialMatchIPAddress(ipv4Address.toString(), searchIP);
    return ret;
  }

  /**
   * Check the searched ip with an ip list or range
   * @param ruleIP
   * @param searchIP
   */
  public static matchIPv4(ruleIP: IPRule, searchIP: string): boolean {
    // checking if can match with list first
    if (ruleIP.list != null) {
      for (let k = 0; k < ruleIP.list.length; k++) {
        const ipAddress = ruleIP.list[k];
        if (this.matchIPv4Helper(ipAddress, searchIP)) {
          return true;
        }
      }
    }

    // checking if can match with range
    if (ruleIP.ranges != null) {
      for (let i = 0; i < ruleIP.ranges.length; i++) {
        const lower = ruleIP.ranges[i].start;
        const upper = ruleIP.ranges[i].end;
        if (this.compareIPAddress(lower, searchIP) <= 0 && this.compareIPAddress(upper, searchIP) >= 0) {
          return true;
        }
        // Check if prefix of the bounds
        if (this.partialMatchIPAddress(lower, searchIP) || this.partialMatchIPAddress(upper, searchIP)) {
          return true;
        }
        // else check if prefix once completed, could be within range
        // Handle incomplete search within bounds
        // We add on 0s to any incomplete search
        // we add a max of 2 zeros
        if (searchIP.endsWith('.')) {
          continue;
        }
        const octets = searchIP.split('.');
        const lastOct = octets[octets.length - 1];
        for (let j = 0; j < 3 - lastOct.length; j++) {
          searchIP += '0';
          if (this.compareIPAddress(lower, searchIP) <= 0 && this.compareIPAddress(upper, searchIP) >= 0) {
            return true;
          }
        }
      }
    }
    return false;
  }

  /**
   * Returns true if the given rule has a port rule matching the protocol and portnumber
   * @param rule            Rule to evaluate
   * @param protocol        Protocol to search for
   * @param portNumber      Port to search for
   */
  public static filterRuleByPort(rule: Rule, protocol: string, portNumber: number): boolean {
    // blank search
    if ((protocol === '' || protocol == null) && portNumber == null) {
      return true;
    }
    const boolMatched = false;
    const ports = rule.ports;

    // If you search only 8,
    // it searches for 8, 80, 800, and 8000
    // will be used for searching if its in a range, but not finished typing
    // ex. user types 8 and range is 8000-9000
    const possiblePortValues = [];
    if (portNumber != null) {
      possiblePortValues.push(portNumber);
      let lastPortNumber = portNumber;
      for (let i = 0; i < 4 - portNumber.toString().length; i++) {
        lastPortNumber *= 10;
        possiblePortValues.push(lastPortNumber);

      }
    }
    // searching rule
    for (let i = 0; i < ports.length; i++) {
      const port = ports[i];
      const portsprotocol = port.protocol;
      // checking protocol
      if (protocol == null || portsprotocol.startsWith(protocol)) {
        // checking port
        if (portNumber == null) {
          return true;
        }
        // checking the rule if its port is in range format
        if (port.ranges) {
          for (let j = 0; j < port.ranges.length; j++) {
            const fromValue = port.ranges[j].start;
            const toValue = port.ranges[j].end;
            if (fromValue.toString().startsWith(portNumber.toString()) ||
              toValue.toString().startsWith(portNumber.toString())) {
              return true;
            }

            for (let k = 0; k < possiblePortValues.length; k++) {
              if (possiblePortValues[k] >= fromValue && possiblePortValues[k] <= toValue) {
                return true;
              }
            }
          }
        } else {
          // checking the rule if its port is in list format
          for (let k = 0; k < port.list.length; k++) {
            if (port.list[k].toString().startsWith(portNumber.toString())) {
              return true;
            }
          }
        }
      }
    }
    return boolMatched;
  }

  public static ipRuleToString(rule: IPRule) {
    let output = '';
    if (rule.ranges != null && rule.ranges.length !== 0) {
      let rList = '';
      for (let i = 0; i < rule.ranges.length; i++) {
        const obj = rule.ranges[i];
        const sourceIP = obj.start;
        const destIP = obj.end;
        const str = sourceIP + ' - ' + destIP;
        if (rList.length === 0) {
          rList = str;
        } else {
          rList = rList + ',' + str;
        }
      }
      output = rList;
    } else {
      let myList = '';
      for (let i = 0; i < rule.list.length; i++) {
        const ipString = rule.list[i];
        if (myList.length === 0) {
          myList = ipString;
        } else {
          myList = myList + ', ' + ipString;
        }
      }
      output = myList;
    }
    return output;
  }

  public static portRulesToString(portRules: PortRule[]) {
    let output = '';
    for (let index = 0; index < portRules.length; index++) {
      const portRule = portRules[index];

      if (index !== 0) {
        output += ', ';
      }
      output += portRule.protocol;

      if (portRule.ranges != null && portRule.ranges.length !== 0) {
        let rList = '';
        for (let i = 0; i < portRule.ranges.length; i++) {
          const obj = portRule.ranges[i];
          const str = obj.start + '-' + obj.end;
          if (rList.length === 0) {
            rList = str;
          } else {
            rList = rList + ', ' + str;
          }
        }
        output = output + ' / ' + rList;
      } else {
        let list = '';
        portRule.list.forEach((rule) => {
          list += rule;
          list += ', ';
        });
        // subtract last comma and space
        list = list.substring(0, list.length - 2);
        output = output + ' / ' + list;
      }
    }
    return output;
  }
}
