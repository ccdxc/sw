import { AfterViewInit, Component, OnDestroy, OnInit, ViewChild, ViewEncapsulation } from '@angular/core';
import { FormControl } from '@angular/forms';
import { BaseComponent } from '@app/components/base/base.component';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { Rule } from '@app/models/frontend/shared/rule.interface';
import { ControllerService } from '@app/services/controller.service';
import { SecurityService } from '@app/services/security.service';
import { Table } from 'primeng/table';
import { IPUtility } from '@app/common/IPUtility';


@Component({
  selector: 'app-sgpolicy',
  templateUrl: './sgpolicy.component.html',
  styleUrls: ['./sgpolicy.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class SgpolicyComponent extends BaseComponent implements OnInit, OnDestroy, AfterViewInit {
  @ViewChild('sgpolicyTable') sgpolicyTurboTable: Table;
  cols: any[];
  sgPolicies: any[]; // Holds all policy objects, for initial use case, there will only be one object

  selectedSGPolices: any[] = [];
  showAll = false;

  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px'
    },
    url: '/assets/images/icons/security/icon-security-policy.svg'
  };

  policyName: string;
  creationTime: number;
  lastModifiedTime: number;
  creator: string;

  ipFormControl: FormControl = new FormControl('', [
  ]);

  portFormControl: FormControl = new FormControl('', [
  ]);

  sgPolicyRules: any[] = []; // Holds all the rules of the first policy, there should be only one policy for inital use case

  constructor(protected _controllerService: ControllerService,
    protected _securityService: SecurityService) {
    super(_controllerService);
  }

  ngOnInit() {
    if (!this._controllerService.isUserLogin()) {
      this._controllerService.publish(Eventtypes.NOT_YET_LOGIN, {});
    } else {
      this._controllerService.publish(Eventtypes.COMPONENT_INIT, { 'component': 'AlerttableComponent', 'state': Eventtypes.COMPONENT_INIT });
      this.getSGPolicies();

      this.cols = [
        { field: 'sourceIPs', header: 'Source IPs' },
        { field: 'destIPs', header: 'Destination IPs' },
        { field: 'action', header: 'Action' },
        { field: 'protocolPort', header: 'Protocol/Ports' },
      ];
      this._controllerService.setToolbarData({

        buttons: [
          {
            cssClass: 'global-button-primary sgpolicy-toolbar-refresh-button',
            text: 'Refresh',
            callback: () => { this.getSGPolicies(); },
          }],
        breadcrumb: [{ label: 'Security', url: '' }, { label: 'Security Policy', url: '' }]
      });
    }
  }

  ipSearch(value: any, filter: any): boolean {
    // value = array of data from the current row
    // filter = value from the filter that will be searched in the value-array
    if (filter === undefined || (filter[0] === '' && filter[1] === '')) {
      return true;
    }

    if (value === undefined || value === null || value.length === 0) {
      return false;
    }

    const ip = filter[0];
    const extractedProtocolPort = IPUtility.extractPortFromString(filter[1]);
    const protocol = extractedProtocolPort.protocol;
    const port = extractedProtocolPort.port;

    return IPUtility.filterRuleByIPv4(value, ip) && IPUtility.filterRuleByPort(value, protocol, port);
  }

  ngAfterViewInit() {
    // Adding our custom filter
    this.sgpolicyTurboTable.filterConstraints['ipSearch'] = this.ipSearch;
  }

  /**
   * Component is about to exit
   */
  ngOnDestroy() {
    // publish event that AppComponent is about to exist
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, { 'component': 'sgpolicyComponent', 'state': Eventtypes.COMPONENT_DESTROY });
  }

  /**
   * Overide super's API
   * It will return this Component name
   */
  getClassName(): string {
    return this.constructor.name;
  }

  getSGPolicies() {
    const payload = '';
    this._securityService.getSGPolicies(payload).subscribe(
      data => {
        this.sgPolicies = data.Items;
        const policy = data.Items[0];
        this.policyName = policy.meta.Name;
        this.creationTime = policy.meta.CreationTime;
        this.lastModifiedTime = policy.meta.ModTime;
        this.creator = policy.meta.Creator;
        const rules: Rule[] = data.Items[0].spec['in-rules'];
        let count = 1;
        // Casting Rule type to any, so that we can set additional properties
        // that will be used in the table display
        rules.forEach((element: any) => {
          element.order = count++;
          element.data = {
            'sourceIP': element.sourceIP,
            'destIP': element.destIP,
            'ports': element.ports,
          };
          element.data.sourceIP.toString = () => IPUtility.ipRuleToString(element.data.sourceIP);
          element.data.destIP.toString = () => IPUtility.ipRuleToString(element.data.destIP);
          element.data.ports.toString = () => IPUtility.portRulesToString(element.data.ports);
        });
        this.sgPolicyRules = rules;
      },
      err => {
        this.successMessage = '';
        this.errorMessage = 'Failed to get SG-Policies! ' + err;
        this.error(err);
      }
    );
  }

  // Used for creating the show all toggle highlighting effect
  computeClass(rowData) {
    if (this.showAll) {
      const ip = this.ipFormControl.value;
      const port = this.portFormControl.value;
      if (this.ipSearch(rowData, [ip, port])) {
        return 'sgpolicy-match';
      }
      return 'sgpolicy-miss';
    }
    return '';
  }

  // Controled by the showall toggle
  toggle(event) {
    this.showAll = event.checked;
    this.filterPolicyRules(event);
  }

  // If the show all toggle is on, we reset the search,
  // Otherwise, we filter the table
  filterPolicyRules(event) {
    if ((this.ipFormControl.value !== '' || this.portFormControl.value !== '') && !this.showAll) {
      const ip = this.ipFormControl.value;
      const port = this.portFormControl.value;
      this.sgpolicyTurboTable.filter([ip, port], 'data', 'ipSearch');
    } else {
      this.sgpolicyTurboTable.filter(null, 'data', 'ipSearch');
    }
  }

  onSGPolicyClick($event, sgpolicy) {
    console.log(this.getClassName() + '.onSGPolicyClick()', sgpolicy);
  }

  /**
  * This API serves html template.
  * It will delete selected alerts
  */
  onSGpolicyDeleteMultiRecords($event) {
    console.log('SgpolicyComponent.onSGpolicyDeleteMultiRecords()', this.selectedSGPolices);
  }

  /**
  * This API serves html template.
  *  It will archive selected alerts
  */
  onSGpolicyArchiveMultiRecords($event) {
    console.log('SgpolicyComponent.onSGpolicyArchiveMultiRecords()', this.selectedSGPolices);
  }

  /**
   * This api serves html template
   */
  onSGpolicyArchiveRecord($event, sgpolicy) {
    console.log('SgpolicyComponent.onSGpolicyArchiveRecord()', sgpolicy);
  }

  /**
   * This api serves html template
   */
  onSGpolicyDeleteRecord($event, sgpolicy) {
    console.log('AlerttableComponent.onAlerttableDeleteRecord()', sgpolicy);
  }


}
