import { environment } from '../../environments/environment';
import { Utility } from './Utility';
import { Rule, RuleAction, IPRule, PortRule, IPRange } from '@app/models/frontend/shared/rule.interface';

export class MockDataUtil {

  static instance: MockDataUtil;
  private constructor() {

  }

  public static getInstance(): MockDataUtil {
    if (!this.instance) {
      this.instance = new MockDataUtil();
    }
    return this.instance;
  }

  public static getGlobalSearchResult(query, data): any {
    const filtered: any[] = [];
    const obj = {
      'indicator': 'pensando',
      'query': query,
      'code': query,
    };
    if (query === 'in') {
      obj['name'] = 'Search By Category - ' + query;
      obj['seachType'] = 'in';
      obj['options'] = ['cluster', 'node', 'workload', 'naples'];
      filtered.push(obj);
    } else if (query === 'is') {
      obj['name'] = 'Search By Type - ' + query;
      obj['seachType'] = 'is';
      obj['options'] = ['workload', 'alert', 'event', 'security'];
      filtered.push(obj);
    } else if (query === 'has') {
      obj['name'] = 'Search By Content - ' + query;
      obj['seachType'] = 'has';
      obj['options'] = ['owner', 'label', 'meta', 'role'];
      filtered.push(obj);
    } else if (query === 'date') {
      obj['name'] = 'Search By Content - ' + query;
      obj['seachType'] = 'date';
      filtered.push(obj);
    } else {
      if (filtered.length === 0) {
        const objMisc = {
          'indicator': 'pensando',
          'name': 'Free Form search - ' + query,
          'query': query,
          'code': query,
        };
        filtered.push(objMisc);
      }
    }
    return filtered;
  }

  public static getWorkloadItems(count): any {
    const items = [];
    for (let i = 0; i < count; i++) {
      const id = 'unique' + i;
      items.push({
        'Kind': 'Endpoint',
        'APIVersion': 'v1',
        'meta': {
          'Name': id,
          'Tenant': 'default',
          'ResourceVersion': '308',
          'UUID': '62ee6857-c971-41b1-bf06-089d45327b85',
          'Labels': {
            'label-1': 'label-1-value',
            'label-2': 'label-2-value'
          },
          'CreationTime': '2018-02-06T02:09:23.504713687Z',
          'ModTime': '2018-02-06T02:09:23.511932951Z'
        },
        'spec': {

        },
        'status': {
          'endpoint-uuid': 'uuid-' + id,
          'workload-uuid': 'workload-' + id,
          'workload-name': 'vm1-' + id,
          'network': 'default',
          'homing-host-addr': '02:02:02:02:02:01',
          'homing-host-name': '02:02:02:02:02:01',
          'ipv4-address': '10.1.0.102/16',
          'ipv4-gateway': '10.1.254.254',
          'mac-address': '00:50:0a:01:00:12',
          'node-uuid': '02:02:02:02:02:01',
          'SecurityGroups': [
            'SG_1'
          ],
          'micro-segment-vlan': 2,
          'workload-attributes': [
            'wl-1',
            'wl-2'
          ]
        }
      });
    }
    return {
      'T': {},
      'ListMeta': {
        'ResourceVersion': '340331'
      },
      'Items': items
    };
  }

  public static getPlotly3DChartData(): any {
    return [
      ['27.80985', '27.71966', '30.4267', '16.66549', '8.815617', '6.628881', '21.83975', '53.34303', '25.66785', '12.827', '0.0', '0.0', '0.0', '0.0', '0.0', '0.0', '0.0', '0.0', '0.0', '0.0', '0.0', '0.0', '0.0', '0.001229679', '0.0'],
      ['49.61936', '48.55022', '33.47752', '30.1086', '18.3516', '10.41339', '6.63927', '26.79797', '63.05717', '69.20554', '68.66396', '95.17499', '134.9879', '48.08919', '54.1941', '6.919659', '21.73871', '0.0', '0.0', '0.0', '0.0', '0.0', '0.0', '3.008948', '5.626141'],
      ['83.08067', '65.21374', '44.80953', '39.96952', '8.658275', '24.81939', '18.97085', '6.63927', '22.1414', '46.76293', '59.0435', '80.03818', '130.3696', '142.5558', '146.3839', '195.1709', '123.1339', '62.04672', '79.62993', '33.2664', '29.77937', '31.409', '19.58994', '5.909858', '7.676256'],
      ['116.6632', '95.27666', '62.47495', '44.12225', '27.5859', '26.08952', '32.89204', '10.88787', '17.074', '13.96517', '33.35762', '59.89862', '96.86325', '140.3777', '99.48143', '132.5253', '176.7414', '136.3122', '139.6978', '57.53643', '54.97282', '132.7418', '42.30355', '33.50574', '63.16226'],
      ['130.414', '116.9964', '77.43523', '59.57512', '48.62691', '30.1605', '43.15014', '17.2044', '41.74483', '33.88744', '47.45282', '39.58476', '75.70494', '154.7261', '96.19411', '135.2341', '158.2698', '201.7952', '173.167', '167.2241', '144.7995', '185.5796', '96.26777', '104.3341', '45.99762'],
      ['150.7206', '133.9056', '104.2153', '77.56929', '60.18013', '52.30802', '62.86014', '56.18116', '60.27227', '61.82613', '57.8355', '50.28058', '58.86466', '87.9361', '102.9473', '89.85069', '137.235', '168.1343', '192.8718', '196.4833', '207.4904', '121.8299', '187.1207', '152.2165', '79.56688'],
      ['220.1871', '152.3412', '102.7393', '106.8925', '91.3286', '64.71007', '104.6657', '79.70141', '81.42432', '84.74799', '78.91689', '63.81641', '57.20374', '58.11092', '76.14089', '89.45549', '105.3089', '95.2046', '196.3499', '194.7966', '165.3432', '185.3841', '179.6626', '198.1988', '227.311'],
      ['156.1536', '151.934', '137.0004', '166.5539', '145.7109', '76.30823', '130.2294', '90.8453', '114.444', '121.122', '107.8275', '80.61302', '55.18837', '52.83869', '57.7844', '60.29967', '86.63255', '58.90624', '144.6611', '182.1884', '171.4047', '160.6566', '221.3898', '191.841', '203.9287'],
      ['148.6416', '160.1139', '186.0706', '175.2381', '116.0653', '84.63686', '114.8494', '98.27675', '102.3234', '145.2741', '168.0053', '66.37824', '78.128', '67.14822', '47.0402', '50.33806', '53.11591', '46.94091', '106.5424', '119.6961', '174.9216', '116.1478', '154.2617', '228.7349', '172.5618'],
      ['203.7845', '179.5327', '219.3173', '185.2815', '106.2662', '99.4324', '106.9873', '80.87243', '101.7878', '153.1797', '130.9597', '198.7651', '108.5582', '83.66798', '64.36799', '39.17583', '29.03865', '49.27053', '57.16653', '73.02113', '100.2733', '118.1078', '142.1604', '168.1041', '177.1462'],
      ['206.0386', '147.6184', '181.7615', '154.5056', '68.69447', '62.52132', '61.89647', '74.7931', '111.031', '204.786', '212.5541', '244.3467', '154.3774', '118.9242', '84.23767', '59.06854', '30.40539', '37.10416', '41.16107', '48.36549', '61.46441', '141.7946', '148.5737', '144.2759', '140.4554'],
      ['107.1618', '170.3943', '120.9154', '83.0433', '53.10596', '46.81647', '55.55682', '75.54661', '119.2309', '227.9242', '165.8122', '294.2474', '319.1686', '150.0681', '162.7181', '74.52159', '39.04902', '17.97011', '32.12764', '33.74652', '50.19171', '65.56351', '67.17937', '110.7436', '123.9905'],
      ['68.36975', '121.8194', '143.1835', '62.61732', '37.92797', '55.76606', '86.80986', '73.4373', '114.0777', '236.3038', '210.2429', '264.3517', '372.8826', '272.9709', '121.3275', '84.93402', '49.23405', '30.93697', '13.8566', '26.2379', '26.08209', '48.84066', '40.69044', '57.65214', '110.346'],
      ['45.3359', '52.58537', '82.40501', '62.33167', '47.95942', '82.4099', '89.27802', '74.11694', '110.5296', '228.3655', '181.1713', '176.4082', '275.4655', '341.1366', '213.1646', '187.1219', '63.27853', '33.39257', '10.91772', '16.3578', '17.18218', '23.13864', '39.74512', '42.63504', '65.12319'],
      ['49.96142', '33.08871', '48.47132', '60.55916', '47.42691', '140.2647', '122.4221', '68.1749', '59.19355', '79.34425', '189.7617', '60.21857', '130.2632', '238.664', '328.482', '123.9673', '111.4215', '44.03077', '12.07177', '6.811293', '8.468093', '18.12932', '26.10166', '27.91891', '34.31887'],
      ['21.89279', '38.40972', '74.71461', '55.92124', '69.20731', '81.26501', '123.9698', '46.24076', '42.47175', '25.93483', '137.3378', '77.41475', '54.93822', '190.2', '285.4489', '103.7027', '101.1956', '55.64542', '22.38254', '6.63927', '6.63927', '10.28531', '14.48469', '15.41052', '24.5278'],
      ['17.02552', '44.24843', '60.0909', '15.17284', '44.95468', '56.45756', '109.0952', '39.93857', '14.63598', '6.944074', '84.65395', '53.16981', '25.49719', '116.8943', '283.8319', '128.986', '40.00962', '78.22423', '24.72105', '6.639271', '6.334467', '6.029663', '8.65873', '8.056102', '9.561069'],
      ['11.74317', '69.5786', '7.073525', '8.248324', '29.17197', '30.42164', '98.41956', '31.21653', '6.944074', '6.944074', '8.677168', '56.16393', '8.047439', '91.48672', '212.815', '165.1283', '59.84565', '14.42782', '6.803666', '8.468093', '6.334467', '6.044627', '3.896037', '3.90283', '3.334991'],
      ['14.75226', '4.019351', '6.089851', '36.68087', '17.91674', '17.28782', '77.61374', '36.88335', '6.944075', '6.944075', '6.956576', '6.949235', '8.084393', '14.0157', '164.549', '249.7054', '74.51253', '9.954997', '4.200841', '6.194273', '5.666687', '5.694764', '3.571392', '3.879774', '5.590495'],
      ['13.6671', '3.050024', '6.53745', '61.93413', '16.25515', '8.302431', '32.49031', '40.02525', '27.74936', '7.553681', '8.468093', '7.531059', '5.115252', '42.29277', '92.29631', '95.39966', '17.06316', '7.768213', '16.46857', '3.591233', '4.272203', '3.739085', '3.896037', '3.936718', '5.487353'],
      ['5.677561', '3.039719', '6.666096', '20.26867', '14.65559', '2.981626', '14.67344', '117.4297', '0.0', '0.0', '0.0', '3.780177', '5.678269', '5.115252', '7.244015', '10.00284', '2.435141', '13.0254', '15.70744', '3.81486', '0.0', '3.896037', '3.896037', '3.968634', '5.909499'],
      ['3.31234', '2.996142', '7.306965', '68.58819', '17.26048', '2.698536', '7.370775', '12.70328', '0.0', '0.0', '0.0', '0.0', '0.0', '0.0', '1.167', '2.39255', '2.287471', '21.73166', '33.96221', '8.600739', '0.0', '0.0', '3.896037', '0.1236256', '5.868994'],
      ['1.156517', '2.967954', '5.73684', '46.49812', '31.22245', '5.886086', '0.03711011', '1.729771', '0.09449376', '0.0', '0.0', '0.0', '0.0', '0.0', '0.0', '0.0', '-0.0003636982', '2.156372', '7.575688', '5.21889', '0.0', '0.0', '1.077756', '3.985531', '5.833817'],
      ['-0.147662', '1.999594', '3.625628', '0.2360095', '46.71704', '5.268358', '0.6423392', '0.0', '0.07732264', '0.0', '0.0', '0.0', '0.0', '0.0', '0.0', '0.0', '0.0', '0.5317867', '-0.04880907', '0.0', '0.0', '0.0', '0.0', '-0.1835741', '3.568177']
    ];
  }

  public static getRandomItemFromList(list): any {
    const item = list[Utility.getRandomInt(0, list.length - 1)];
    return item;
  }

  public static getRandomLineSplineData(lenPlotly: number): any {
    const mockPlotlyX = this.generateRandomPlotlyMockData(lenPlotly);
    const mockPlotlyY = this.generateRandomPlotlyMockData(lenPlotly);

    mockPlotlyX.sort(function(a, b) { return a - b; });
    const xyObj = {
      x: mockPlotlyX,
      y: mockPlotlyY
    };
    return xyObj;
  }

  public static generateRandomPlotlyMockData(len: number) {
    const chartData = [];
    for (let i = 0; i < len; i++) {
      chartData.push(Math.floor(Math.random() * 100));
    }
    return chartData;
  }

  public static getRandomWorkloadStyle(): String {
    const options = ['positive', 'negative'];
    return this.getRandomItemFromList(options);
  }

  public static getMockedData(url: string, method: string, eventpayload: any): any {
    const reqID = this.getMockRequestID(url, method, eventpayload);
    switch (reqID) {
      case 'login':
        return this.mockLogin();
      case 'endpoints':
        return this.getWorkloadItems(8);
      case 'globalsearch':
        return this.mockGlobalSearch();
      case 'alertlist':
        return this.mockAlertlist();
      case 'alerttable':
        return this.mockAlerttable();
      case 'workloadwidget':
        return {
          totalworkloads: this.mockWorkloadWidget(8, 1000, 10000),
          newworkloads: this.mockWorkloadWidget(8, 2, 400),
          unprotectedworkloads: this.mockWorkloadWidget(8, 0, 50),
          workloadalerts: this.mockWorkloadWidget(8, 0, 50),
        };
      case 'workloadwidget-totalworkloads':
        return this.mockWorkloadWidget(8, 1000, 10000);
      case 'workloadwidget-newworkloads':
        return this.mockWorkloadWidget(8, 2, 400);
      case 'workloadwidget-unprotectedworkloads':
        return this.mockWorkloadWidget(8, 0, 50);
      case 'workloadwidget-workloadalerts':
        return this.mockWorkloadWidget(8, 0, 50);
      case 'dsbdworkloadwidget':
        const endpointPercent = Utility.getRandomIntList(3, 20, 30);
        return {
          veniceEndpointCount: Utility.getRandomInt(200000, 500000),
          veniceNewEndpointCount: Utility.getRandomInt(200, 2000),
          endpointList: Utility.getRandomIntList(3, 30, 41),
          endpointPercent: endpointPercent,
          endpointPercentCompliment: Utility.getComplimentaryList(endpointPercent),
        };
      case 'sgpolicy':
        return this.mockSGPolicies();
      default:
        return {};
    }
  }

  public static mockLogin() {
    return {
      'kind': 'User',
      'api-version': 'v1',
      'meta': {
        'name': 'Liz',
        'tenant': 'default',
        'resource-version': '10028',
        'uuid': 'fc103fd3-6ba6-4193-aeed-419419b1b4c3',
        'creation-time': '2018-04-16T23:51:26.422201836Z',
        'mod-time': '2018-04-16T23:51:26.422203465Z',
        'self-link': '/v1/auth/default/users/Liz'
      },
      'spec': {
        'fullname': 'Liz Claire',
        'email': 'vishaljain@pensando.io',
        'type': 'LOCAL'
      },
      'status': {}
    };
  }

  public static mockGlobalSearch() {
    return [
      { login: 'John' },
      { login: 'Doe' }
    ];
  }

  public static buidCreateAndModDates(): any {
    const monment = Utility.getMomentJS();
    const minusDays = Utility.getRandomInt(1, 30);
    const cDate = monment().subtract(minusDays, 'd');
    const addDays = Utility.getRandomInt(0, minusDays);
    const mDate = monment(cDate).add(addDays, 'd');
    mDate.add(addDays, 'h');
    mDate.subtract(minusDays + 'm');
    const obj = {
      cdate: cDate,
      mdate: mDate
    };
    return obj;
  }

  public static buildUUID(): string {
    return Utility.s4() + Utility.s4() + '-' + Utility.s4() + '-' + Utility.s4() + '-' + Utility.s4() + Utility.s4() + Utility.s4();
  }

  public static buildRandomLabelObject(len?: number): any {
    const labelObj = {};
    const myLen = (len) ? len : Utility.getRandomInt(1, 10);
    for (let i = 0; i < myLen; i++) {
      const key = Utility.s4();
      const value = Utility.s4();
      labelObj[key] = value;
    }
    return labelObj;
  }

  public static buildDefaultOutRule(): any {
    return {
      'ports': 'any',
      'action': 'allow'
    };
  }
  /**
   * {
            'ports': 'tcp/80-90', // or imcp:
            'action': 'allow'
      },
   */
  public static buildInRulePortData(): PortRule[] {
    // const protocols = ['BGP', 'DHCP', 'DNS', 'FTP', 'HTTP', 'IMAP', 'LDAP', 'MGCP', 'MQTT', 'NNTP', 'NTP', 'POP',  'RTP', 'RTSP', 'RIP', 'SIP', 'SMTP', 'SNMP', 'SSH', 'Telnet', 'TLS/SSL', 'XMPP', 'TCP', 'UDP', 'DCCP', 'SCTP', 'RSVP','ICMP'];
    const protocols = ['TCP', 'UDP'];
    const protocol = this.getRandomItemFromList(protocols);
    let portList = null;
    let rangeList = null;

    const isRanged = this.getRandomItemFromList([true, false]);
    if (isRanged) {
      rangeList = [];
      const rLen = Utility.getRandomInt(1, 10);
      for (let i = 0; i < rLen; i++) {
        const fromNumber = Utility.getRandomInt(1, 1000);
        const toNumber = Utility.getRandomInt(fromNumber, 1000);
        const range = {};
        range['start'] = fromNumber;
        range['end'] = toNumber;
        rangeList.push(range);
      }
    } else {
      portList = [];
      let len = Utility.getRandomInt(1, 10);
      if (protocol === 'ICMP') {
        len = 0; // for ICMP to no port number
      }
      for (let i = 0; i < len; i++) {
        portList.push(Utility.getRandomInt(1, 1000));
      }
      portList = portList.sort();
    }

    const portData: PortRule = {
      'protocol': protocol,
      'ranges': rangeList,
      'list': portList,
    };
    return [portData];
  }

  static getIP_oct1(): number {
    return (Math.floor(Math.random() * 255) + 1);
  }

  static getIP_oct2(): number {
    return (Math.floor(Math.random() * 255) + 0);
  }

  static getIPv4Address(buildSub: boolean = false): string {
    let ipStr = '';
    ipStr += this.getIP_oct1();
    ipStr += '.' + this.getIP_oct2();
    const isOct3Wildcard = this.getRandomItemFromList([true, false]);
    if (isOct3Wildcard) {
      ipStr += '.*';
    } else {
      ipStr += '.' + this.getIP_oct2();
    }
    let isOct4Wildcard = isOct3Wildcard;
    if (!isOct3Wildcard) {
      isOct4Wildcard = this.getRandomItemFromList([true, false]);
    }
    if (isOct4Wildcard) {
      ipStr += '.*';
    } else {
      ipStr += '.' + this.getIP_oct2();
    }
    const sub = (buildSub && !isOct3Wildcard && !isOct4Wildcard) ? Utility.getRandomInt(1, 24).toString() : '';
    ipStr += sub;

    return ipStr;
  }



  public static _increaseOct(value: any): any {
    value = Utility.getRandomInt(value + 1, 255);
    return value;
  }

  /**
   * For building the destIP in a range format
   * Makes sure that the dest is greater than the given source
   */
  public static _builddestIP(sourceIP: string): string {
    const octs = sourceIP.split('.');
    const destOcts = Utility.getLodash().cloneDeep(octs);
    if (octs[2] === '*' && octs[3] === '*') {
      destOcts[1] = this._increaseOct(octs[1]);
    } else if (octs[3] === '*') {
      destOcts[2] = this._increaseOct(octs[2]);
    } else {
      const isIncOc4 = this.getRandomItemFromList([true, false]);
      if (isIncOc4) {
        destOcts[3] = this._increaseOct(octs[3]);
      }
      const isIncOc3 = this.getRandomItemFromList([true, false]);
      if (isIncOc3 && !isIncOc4) {
        destOcts[2] = this._increaseOct(destOcts[2]);
      }
    }
    return destOcts.join('.');

  }
  public static buildIPData(): IPRule {
    let ipList = null;
    let rangeList = null;

    const isRanged = this.getRandomItemFromList([true, false]);
    if (isRanged) {
      rangeList = [];
      // We don't allow submask for ranges currently
      // in our searching functionality, searching will
      // probably be moved to the backend in the future
      const sourceIP = this.getIPv4Address(false);
      const destIP = this._builddestIP(sourceIP);
      const ip: IPRange = {
        start: sourceIP,
        end: destIP
      };
      rangeList.push(ip);
    } else {
      ipList = [];
      const len = Utility.getRandomInt(1, 5);
      for (let i = 0; i < len; i++) {
        const isSub = this.getRandomItemFromList([true, false]);
        ipList.push(this.getIPv4Address(isSub));
      }
      ipList = ipList.sort();
    }
    const ipData: IPRule = {
      'list': ipList,
      'ranges': rangeList,
    };
    return ipData;
  }

  public static buildInRule(isPermit: boolean = true): Rule {
    const sourceIP: IPRule = this.buildIPData();
    const destIP: IPRule = this.buildIPData();
    const ret = {
      'sourceIP': sourceIP,
      'destIP': destIP,
      'ports': this.buildInRulePortData(),
      'action': (isPermit) ? RuleAction.permit : RuleAction.deny
    };
    return ret;
  }


  public static buildmockSGPolicyObject() {
    const objName = (name) ? name : 'sgpolicy-' + Utility.s4();
    const cmDate = this.buidCreateAndModDates();
    const cDate = cmDate.cdate;
    const mDate = cmDate.mdate;
    const uuid = this.buildUUID();

    const inRuleLen = 50;
    const inRules: Rule[] = [];

    for (let i = 0; i < inRuleLen; i++) {
      const isPermit = (i < inRuleLen - 1);
      // Two rules are always fixed for demo purposes
      if (i === 3) {
        const fixedRule: Rule = {
          sourceIP: {
            ranges: [{
              start: '10.1.1.13',
              end: '10.1.1.24'
            }],
          },
          destIP: {
            list: ['11.34.23.44']
          },
          ports: [{
            protocol: 'TCP',
            ranges: [{
              start: 8000,
              end: 9000
            }]
          }],
          action: RuleAction.permit
        };

        inRules.push(fixedRule);
        continue;
      }
      if (i === 23) {
        const fixedRule: Rule = {
          sourceIP: {
            list: ['198.62.1.0/24']
          },
          destIP: {
            list: ['10.1.1.0/24']
          },
          ports: [{
            protocol: 'UDP',
            list: [8080]
          }],
          action: RuleAction.permit
        };
        inRules.push(fixedRule);
        continue;
      }
      inRules.push(this.buildInRule(isPermit));
    }
    return {
      'Kind': 'Sgpolicy',
      'APIVersion': 'v1',
      'meta': {
        'Name': objName,
        'Creator': 'Ravi Gadde',
        'Tenant': 'default',
        'ResourceVersion': Utility.getRandomInt(10000, 34550),
        'UUID': uuid,
        'CreationTime': cDate.toISOString(),
        'ModTime': mDate.toISOString(),
        'Labels': this.buildRandomLabelObject(),
        'SelfLink': '/v1/sgpolicy/default/sgpolicy/' + objName
      },
      'spec': {
        'attach-groups': [
          'SG3',
          'SG0'
        ],
        'in-rules': inRules,
        'out-rules': [
          this.buildDefaultOutRule()
        ]
      },
      'status': {}
    };
  }

  public static mockSGPolicies() {
    const len = 1;
    const sgpolices = [];
    for (let i = 0; i < len; i++) {
      sgpolices.push(this.buildmockSGPolicyObject());
    }

    return {
      'T': {},
      'ListMeta': {},
      'Items': sgpolices
    };
  }

  public static mockWorkloadWidget(dataPoints, randLower, randUpper) {
    return {
      data: this.getRandomLineSplineData(dataPoints),
      label: Utility.getRandomInt(randLower, randUpper),
    };
  }

  public static getTopPolicyHealthRecords(len: number, start: number, end: number, idPattern: string, namePattern: string): any {
    let topViolatedPolicies = [];
    for (let i = 0; i < len; i++) {
      const s4 = Utility.s4();
      const obj = {
        id: idPattern + s4,
        name: namePattern + s4,
        times: Utility.getRandomInt(start, end)
      };
      topViolatedPolicies.push(obj);
    }
    topViolatedPolicies = Utility.getLodash().sortBy(topViolatedPolicies, [function(o) { return o.times; }]);
    topViolatedPolicies = topViolatedPolicies.reverse();
    return topViolatedPolicies;
  }

  public static getMockRequestID(url: string, method: string, eventpayload: any): string {
    if (url.indexOf('login') >= 0) {
      return 'login';
    } else if (url.indexOf('nodes') >= 0) {
      return 'globalsearch';
    } else if (url.indexOf('endpoints') >= 0) {
      return 'endpoints';
    } else if (url.indexOf('alertlist') >= 0) {
      return 'alertlist';
    } else if (url.indexOf('alerttable') >= 0) {
      return 'alerttable';
    } else if (url.indexOf('sgpolicy') >= 0) {
      return 'sgpolicy';
    } else if (url.indexOf('workloadwidgetById') >= 0) {
      if (url.indexOf('totalworkloads') >= 0) {
        return 'workloadwidget-totalworkloads';
      } else if (url.indexOf('newworkloads') >= 0) {
        return 'workloadwidget-newworkloads';
      } else if (url.indexOf('unprotectedworkloads') >= 0) {
        return 'workloadwidget-unprotectedworkloads';
      } else if (url.indexOf('workloadalerts') >= 0) {
        return 'workloadwidget-workloadalerts';
      }
    } else if (url.indexOf('dsbdworkloadwidget') >= 0) {
      return 'dsbdworkloadwidget';
    } else if (url.indexOf('workloadwidget') >= 0) {
      return 'workloadwidget';
    }
    return 'dummy';
  }

  public static mockAlerttable(): any {
    const list = [];
    const total = 120;
    const len = Utility.getRandomInt(1, total);
    for (let i = 0; i < len; i++) {
      const date = Utility.getRandomDate(Utility.getRandomInt(0, 10), Utility.getRandomInt(0, 24), Utility.getRandomInt(0, 24));
      const severity = this.getRandomItemFromList(['critical', 'warning', 'info']);
      const type = this.getRandomItemFromList(['endpoint', 'policy', 'naples', 'telemetry', 'security', 'cluster', 'diagnostics']);
      const name = type + ' ' + Utility.getRandomInt(0, 20000);
      const message = this.getRandomItemFromList(['Authentication Failure User authentication failed',
        'Cluster Security DTLS configuration changed',
        'Security Policy policy-SC-0994 is recovered',
        'System Update Version 1.13 is available',
        'Orchestration Alert 1 duplicate IP address found'
      ]);
      const obj = {
        id: i,
        date: date,
        type: type,
        name: name,
        severity: severity,
        message: message
      };
      list.push(obj);
    }
    return list;
  }

  public static mockAlertlist(): any {
    const total = 20;
    const len = Utility.getRandomInt(1, total);
    const list = [];
    for (let i = 0; i < len; i++) {
      const user = this.getRandomItemFromList(['admin', 'operator', 'super-admin', 'security-admin']);
      const severity = this.getRandomItemFromList(['critical', 'warning', 'info']);
      const source = this.getRandomItemFromList(['endpoint', 'policy', 'naples', 'telemetry', 'security', 'cluster', 'diagnostics']);
      const time = Utility.getRandomInt(1, 200);
      const obj = {
        id: i,
        label: 'problem' + i,
        description: 'something is wrong ' + i,
        user: user,
        severity: severity,
        time: time + ' mininutes ago',
        source: source
      };
      list.push(obj);
    }
    return list;
  }

}
