import { IPUtility } from '@app/common/IPUtility';
import { Rule, RuleAction } from '@app/models/frontend/shared/rule.interface';


describe('IPUtility', () => {
  const listReference: Rule = {
    sourceIP: {
      list: ['192.168.10.2', '200.244.100.1']
    },
    destIP: {
      list: ['10.10.10.10']
    },
    ports: [{
      protocol: 'TCP',
      list: [8000, 9000, 9050]
    }],
    action: RuleAction.permit
  };

  const maskReference: Rule = {
    sourceIP: {
      list: ['192.168.10.0/24']
    },
    destIP: {
      list: ['10.10.10.10/22']
    },
    ports: [{
      protocol: 'TCP',
      list: [8000]
    }],
    action: RuleAction.permit
  };

  const wildcardReference: Rule = {
    sourceIP: {
      list: ['192.*.10.4']
    },
    destIP: {
      list: ['*.10.*.*']
    },
    ports: [{
      protocol: 'TCP',
      list: [8000]
    }],
    action: RuleAction.permit
  };

  const rangeReference: Rule = {
    sourceIP: {
      ranges: [{
        start: '192.168.10.20',
        end: '192.168.10.83'
      }]
    },
    destIP: {
      list: ['10.10.10.10']
    },
    ports: [{
      protocol: 'TCP',
      ranges: [{
        start: 8000,
        end: 9200
      },
      {
        start: 9050,
        end: 9070
      }]
    }],
    action: RuleAction.permit
  };

  it('extractPortFromString', () => {
    let portString: string;
    let ret;
    let expected;

    portString = 'TCP 8000';
    expected = { port: 8000, protocol: 'TCP' };
    ret = IPUtility.extractPortFromString(portString);
    expect(ret.port).toBe(expected.port);
    expect(ret.protocol).toBe(expected.protocol);

    // Should ignore slashes
    portString = 'TCP/8000';
    expected = { port: 8000, protocol: 'TCP' };
    ret = IPUtility.extractPortFromString(portString);
    expect(ret.port).toBe(expected.port);
    expect(ret.protocol).toBe(expected.protocol);

    // Port protocol order should not matter
    portString = '8000 TCP';
    expected = { port: 8000, protocol: 'TCP' };
    ret = IPUtility.extractPortFromString(portString);
    expect(ret.port).toBe(expected.port);
    expect(ret.protocol).toBe(expected.protocol);

    // spacing shouldnt matter
    portString = '8000TCP';
    expected = { port: 8000, protocol: 'TCP' };
    ret = IPUtility.extractPortFromString(portString);
    expect(ret.port).toBe(expected.port);
    expect(ret.protocol).toBe(expected.protocol);
  });

  it('generateFullIP', () => {
    let ip;
    let ret;
    let expected;

    ip = '192';
    expected = '192.*.*.*';
    ret = IPUtility.generateFullIP(ip);
    expect(ret).toBe(expected);

    // Ending in a dot shouldnt change the result
    ip = '192.';
    expected = '192.*.*.*';
    ret = IPUtility.generateFullIP(ip);
    expect(ret).toBe(expected);

    // blank input
    ip = '';
    expected = '*.*.*.*';
    ret = IPUtility.generateFullIP(ip);
    expect(ret).toBe(expected);

    // Fully formed IP shouldnt be affected
    ip = '10.1.1.2';
    expected = '10.1.1.2';
    ret = IPUtility.generateFullIP(ip);
    expect(ret).toBe(expected);

    // Incorrectly formed IP with extra octets shouldnt be affected
    ip = '10.1.1.2.2.2';
    expected = '10.1.1.2.2.2';
    ret = IPUtility.generateFullIP(ip);
    expect(ret).toBe(expected);
  });

  it('partialMatchIPAddress', () => {
    let refAddr;
    let searchAddr;
    let ret;

    // First octet prefix matching
    refAddr = '192.10.1.20';
    searchAddr = '19';
    ret = IPUtility.partialMatchIPAddress(refAddr, searchAddr);
    expect(ret).toBeTruthy();

    // First three octets prefix match
    refAddr = '192.10.1.20';
    searchAddr = '192.10.1';
    ret = IPUtility.partialMatchIPAddress(refAddr, searchAddr);
    expect(ret).toBeTruthy();

    // Dot at the end shouldn't affect the result
    refAddr = '192.10.1.20';
    searchAddr = '192.10.1.';
    ret = IPUtility.partialMatchIPAddress(refAddr, searchAddr);
    expect(ret).toBeTruthy();

    // first octet doesn't match
    refAddr = '192.10.1.20';
    searchAddr = '19.10.1.';
    ret = IPUtility.partialMatchIPAddress(refAddr, searchAddr);
    expect(ret).toBeFalsy();

    // Wildcards should match to everything
    refAddr = '192.10.1.20';
    searchAddr = '*.10.1.';
    ret = IPUtility.partialMatchIPAddress(refAddr, searchAddr);
    expect(ret).toBeTruthy();

    refAddr = '192.10.1.20';
    searchAddr = '*.*.*.*';
    ret = IPUtility.partialMatchIPAddress(refAddr, searchAddr);
    expect(ret).toBeTruthy();

    // Exact match
    refAddr = '192.10.1.20';
    searchAddr = '192.10.1.20';
    ret = IPUtility.partialMatchIPAddress(refAddr, searchAddr);
    expect(ret).toBeTruthy();

    // searchAddr being longer should fail
    refAddr = '192.10.1.20';
    searchAddr = '192.10.1.200';
    ret = IPUtility.partialMatchIPAddress(refAddr, searchAddr);
    expect(ret).toBeFalsy();
  });

  it('compareIPAddress', () => {
    let addr1;
    let addr2;
    let ret;

    addr1 = '10.1.1.2';
    addr2 = '10.1.1.14';
    ret = IPUtility.compareIPAddress(addr1, addr2);
    expect(ret).toBeLessThan(0);

    // Swapping should return the opposite result
    ret = IPUtility.compareIPAddress(addr2, addr1);
    expect(ret).toBeGreaterThan(0);

    // Wildcard is treated as equal
    addr1 = '10.*.1.2';
    addr2 = '10.1.1.14';
    ret = IPUtility.compareIPAddress(addr1, addr2);
    expect(ret).toBeLessThan(0);

    // IP doesnt need to be fully coplete
    addr1 = '10.1.';
    addr2 = '10.1.1.14';
    ret = IPUtility.compareIPAddress(addr1, addr2);
    expect(ret).toEqual(0);

    addr1 = '10.3.';
    addr2 = '10.1.1.14';
    ret = IPUtility.compareIPAddress(addr1, addr2);
    expect(ret).toBeGreaterThan(0);

    // Support mask in CIDR form
    addr1 = '10.*.1.2';
    addr2 = '10.1.1.14/24';
    ret = IPUtility.compareIPAddress(addr1, addr2);
    expect(ret).toEqual(0);

    // Test equal ips
    addr1 = '10.1.1.14';
    addr2 = '10.1.1.14';
    ret = IPUtility.compareIPAddress(addr1, addr2);
    expect(ret).toEqual(0);

    // Test equal ips
    addr1 = '10.1.0.10/16';
    addr2 = '10.1.1.0/16';
    ret = IPUtility.compareIPAddress(addr1, addr2);
    expect(ret).toEqual(0);
  });

  it('filterRuleByIPv4 and matchIPv4', () => {
    let ip;
    let ret;

    // exact source match search
    ip = '192.168.10.2';
    ret = IPUtility.filterRuleByIPv4(listReference, ip);
    expect(ret).toBeTruthy();

    // exact dest match search
    ip = '10.10.10.10';
    ret = IPUtility.filterRuleByIPv4(listReference, ip);
    expect(ret).toBeTruthy();


    // false search
    ip = '10.10.10.2';
    ret = IPUtility.filterRuleByIPv4(listReference, ip);
    expect(ret).toBeFalsy();

    // prefix search valid
    ip = '192.16';
    ret = IPUtility.filterRuleByIPv4(listReference, ip);
    expect(ret).toBeTruthy();

    // prefix search invalid
    ip = '10.1.';
    ret = IPUtility.filterRuleByIPv4(listReference, ip);
    expect(ret).toBeFalsy();

    // invalid ip
    ip = '10.10.a.as';
    ret = IPUtility.filterRuleByIPv4(listReference, ip);
    expect(ret).toBeFalsy();

    ip = 'asd';
    ret = IPUtility.filterRuleByIPv4(listReference, ip);
    expect(ret).toBeFalsy();

    ip = '1000.1.1.1';
    ret = IPUtility.filterRuleByIPv4(listReference, ip);
    expect(ret).toBeFalsy();

    ip = '10.1.1.1.1';
    ret = IPUtility.filterRuleByIPv4(listReference, ip);
    expect(ret).toBeFalsy();

    ip = '10.1..1.1';
    ret = IPUtility.filterRuleByIPv4(listReference, ip);
    expect(ret).toBeFalsy();

    // mask search IP
    ip = '192.168.10.200';
    ret = IPUtility.filterRuleByIPv4(maskReference, ip);
    expect(ret).toBeTruthy();

    // mask search Dest
    ip = '10.10.11.100';
    ret = IPUtility.filterRuleByIPv4(maskReference, ip);
    expect(ret).toBeTruthy();

    // wildcard search
    ip = '192.200.10.4';
    ret = IPUtility.filterRuleByIPv4(wildcardReference, ip);
    expect(ret).toBeTruthy();

    // range search
    ip = '192.168.10.80';
    ret = IPUtility.filterRuleByIPv4(rangeReference, ip);
    expect(ret).toBeTruthy();

    // range search
    ip = '192.168.10.80';
    ret = IPUtility.filterRuleByIPv4(rangeReference, ip);
    expect(ret).toBeTruthy();

    // on boundary
    ip = '192.168.10.20';
    ret = IPUtility.filterRuleByIPv4(rangeReference, ip);
    expect(ret).toBeTruthy();

    ip = '192.168.10.83';
    ret = IPUtility.filterRuleByIPv4(rangeReference, ip);
    expect(ret).toBeTruthy();

    // out of boundary
    ip = '192.168.10.86';
    ret = IPUtility.filterRuleByIPv4(rangeReference, ip);
    expect(ret).toBeFalsy();
  });

  it('cdirToNetmask', () => {
    let bitCount;
    let expected;
    let ret;

    bitCount = 0;
    expected = '0.0.0.0';
    ret = IPUtility.cdirToNetmask(bitCount);
    expect(ret).toEqual(expected);

    // negative numbers are invalid, treated as 0
    bitCount = -5;
    expected = '0.0.0.0';
    ret = IPUtility.cdirToNetmask(bitCount);
    expect(ret).toEqual(expected);

    bitCount = 32;
    expected = '255.255.255.255';
    ret = IPUtility.cdirToNetmask(bitCount);
    expect(ret).toEqual(expected);

    // over 32 is inavlid, treated as 32
    bitCount = 64;
    expected = '255.255.255.255';
    ret = IPUtility.cdirToNetmask(bitCount);
    expect(ret).toEqual(expected);

    bitCount = 26;
    expected = '255.255.255.192';
    ret = IPUtility.cdirToNetmask(bitCount);
    expect(ret).toEqual(expected);
  });

  it('matchIPv4Helper', () => {
    let searchIP;
    let ref;
    let ret;

    // Exact match
    searchIP = '10.1.1.3';
    ref = '10.1.1.3';
    ret = IPUtility.matchIPv4Helper(ref, searchIP);
    expect(ret).toBeTruthy();

    // partial match
    searchIP = '10.1';
    ref = '10.1.1.3';
    ret = IPUtility.matchIPv4Helper(ref, searchIP);
    expect(ret).toBeTruthy();

    searchIP = '10.11';
    ref = '10.1.1.3';
    ret = IPUtility.matchIPv4Helper(ref, searchIP);
    expect(ret).toBeFalsy();

    searchIP = '10.1.1.33';
    ref = '10.1.1.3';
    ret = IPUtility.matchIPv4Helper(ref, searchIP);
    expect(ret).toBeFalsy();
  });

  it('filterRuleByPort', () => {
    let protocol;
    let port;
    let ret;

    // protocol search
    protocol = 'TCP';
    port = 8000;
    ret = IPUtility.filterRuleByPort(listReference, protocol, port);
    expect(ret).toBeTruthy();

    protocol = '';
    port = 8000;
    ret = IPUtility.filterRuleByPort(listReference, protocol, port);
    expect(ret).toBeTruthy();

    protocol = null;
    port = 8000;
    ret = IPUtility.filterRuleByPort(listReference, protocol, port);
    expect(ret).toBeTruthy();

    protocol = 'TCP';
    port = null;
    ret = IPUtility.filterRuleByPort(listReference, protocol, port);
    expect(ret).toBeTruthy();

    // partial match
    protocol = 'TC';
    port = null;
    ret = IPUtility.filterRuleByPort(listReference, protocol, port);
    expect(ret).toBeTruthy();

    protocol = null;
    port = 80;
    ret = IPUtility.filterRuleByPort(listReference, protocol, port);
    expect(ret).toBeTruthy();

    protocol = 'TC';
    port = 80;
    ret = IPUtility.filterRuleByPort(listReference, protocol, port);
    expect(ret).toBeTruthy();

    // range search
    protocol = 'TCP';
    port = 8500;
    ret = IPUtility.filterRuleByPort(rangeReference, protocol, port);
    expect(ret).toBeTruthy();

    // wrong protocol
    protocol = 'UDP';
    port = 8000;
    ret = IPUtility.filterRuleByPort(listReference, protocol, port);
    expect(ret).toBeFalsy();

    // wrong port
    protocol = 'TCP';
    port = 6000;
    ret = IPUtility.filterRuleByPort(listReference, protocol, port);
    expect(ret).toBeFalsy();
  });

  it('ipRuleToString', () => {
    let expected;
    let ret;

    expected = '192.168.10.2, 200.244.100.1';
    ret = IPUtility.ipRuleToString(listReference.sourceIP);
    expect(ret).toEqual(expected);

    expected = '192.168.10.0/24';
    ret = IPUtility.ipRuleToString(maskReference.sourceIP);
    expect(ret).toEqual(expected);

    expected = '192.*.10.4';
    ret = IPUtility.ipRuleToString(wildcardReference.sourceIP);
    expect(ret).toEqual(expected);

    expected = '192.168.10.20 - 192.168.10.83';
    ret = IPUtility.ipRuleToString(rangeReference.sourceIP);
    expect(ret).toEqual(expected);
  });

  it('portRulesToString', () => {
    let expected;
    let ret;

    expected = 'TCP / 8000, 9000, 9050';
    ret = IPUtility.portRulesToString(listReference.ports);
    expect(ret).toEqual(expected);

    expected = 'TCP / 8000-9200, 9050-9070';
    ret = IPUtility.portRulesToString(rangeReference.ports);
    expect(ret).toEqual(expected);

  });
});
