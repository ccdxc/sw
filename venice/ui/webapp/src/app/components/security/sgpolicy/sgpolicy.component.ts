import { AfterViewInit, Component, ElementRef, OnDestroy, OnInit, ViewChild, ViewEncapsulation } from '@angular/core';
import { FormControl } from '@angular/forms';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { IPUtility } from '@app/common/IPUtility';
import { BaseComponent } from '@app/components/base/base.component';
import { LazyrenderComponent } from '@app/components/shared/lazyrender/lazyrender.component';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { ControllerService } from '@app/services/controller.service';
import { SecurityService } from '@app/services/generated/security.service';
import { IApiStatus, ISecuritySGPolicy, ISecuritySGRule, SecuritySGPolicy } from '@sdk/v1/models/generated/security';
import { Table } from 'primeng/table';
import { Icon } from '@app/models/frontend/shared/icon.interface';

class SecuritySGRuleWrapper {
  order: number;
  rule: ISecuritySGRule;
}


@Component({
  selector: 'app-sgpolicy',
  templateUrl: './sgpolicy.component.html',
  styleUrls: ['./sgpolicy.component.scss'],
  encapsulation: ViewEncapsulation.None,
})
export class SgpolicyComponent extends BaseComponent implements OnInit, OnDestroy, AfterViewInit {
  @ViewChild('sgpolicyTable') sgpolicyTurboTable: Table;
  @ViewChild(LazyrenderComponent) lazyRenderWrapper: LazyrenderComponent;
  viewInitComplete: boolean = false;

  subscriptions = [];

  cols: any[];
  // Holds all policy objects, for initial use case, there will only be one object
  sgPolicies: ReadonlyArray<ISecuritySGPolicy> = [];
  sgPoliciesEventUtility: HttpEventUtility;

  // Used for the table - when true there is a loading icon displayed
  loading: boolean = false;

  // show all toggle value
  showAll = false;

  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px'
    },
    url: '/assets/images/icons/security/icon-security-policy-black.svg'
  };

  policyIcon: Icon = {
    margin: {
      top: '0px',
      left: '0px',
    },
    svgIcon: 'policy'
  };

  // Current policy that is being displayed
  selectedPolicy: ISecuritySGPolicy;

  // TODO: Update with actual creator
  creator = 'rsikdar';

  ipFormControl: FormControl = new FormControl('', [
  ]);

  portFormControl: FormControl = new FormControl('', [
  ]);

  // Holds the policy rules that are currently displayed in the table
  sgPolicyRules: ReadonlyArray<SecuritySGRuleWrapper[]> = [];

  constructor(protected _controllerService: ControllerService,
    private elRef: ElementRef,
    protected _securityService: SecurityService
  ) {
    super(_controllerService);
  }

  ngOnInit() {
    this._controllerService.publish(Eventtypes.COMPONENT_INIT, { 'component': 'AlerttableComponent', 'state': Eventtypes.COMPONENT_INIT });
    this.getSGPolicies();

    this.cols = [
      { field: 'sourceIPs', header: 'Source IPs' },
      { field: 'destIPs', header: 'Destination IPs' },
      { field: 'action', header: 'Action' },
      { field: 'protocolPort', header: 'Protocol/Ports' },
    ];
    this._controllerService.setToolbarData({

      buttons: [],
      breadcrumb: [{ label: 'Security', url: '' }, { label: 'Security Policy', url: '' }]
    });
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
    this.viewInitComplete = true;
  }


  /**
   * Component is about to exit
   */
  ngOnDestroy() {
    // publish event that AppComponent is about to be destroyed
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, { 'component': 'sgpolicyComponent', 'state': Eventtypes.COMPONENT_DESTROY });
    this.subscriptions.forEach(subscription => {
      subscription.unsubscribe();
    });
  }

  /**
   * Overide super's API
   * It will return this Component name
   */
  getClassName(): string {
    return this.constructor.name;
  }

  getSGPolicies() {
    this.loading = true;
    this.sgPoliciesEventUtility = new HttpEventUtility();
    this.sgPolicies = this.sgPoliciesEventUtility.array;
    const subscription = this._securityService.WatchSGPolicy().subscribe(
      response => {
        const body: any = response.body;
        this.sgPoliciesEventUtility.processEvents(body);
        // only look at the first policy
        if (this.sgPolicies.length > 0) {
          // Set sgpolicyrules
          this.selectedPolicy = new SecuritySGPolicy(this.sgPolicies[0]);
          this.sgPolicyRules = this.addOrderRanking(this.selectedPolicy.spec.rules);
        }
        this.loading = false;
      },
      error => {
        // TODO: Error handling
        if (error.body instanceof Error) {
          console.error('Monitoring service returned code: ' + error.statusCode + ' data: ' + <Error>error.body);
        } else {
          console.error('Monitoring service returned code: ' + error.statusCode + ' data: ' + <IApiStatus>error.body);
        }
      }
    );
    this.subscriptions.push(subscription);
  }

  /**
   * Returns the table's current scroll amount
   */
  getTableScroll() {
    return this.elRef.nativeElement.querySelector('.ui-table-scrollable-body').scrollTop;
  }

  /**
   * Adds a wrapper object around the rules to store ordering
   * so that they can easily be uniquely identified
   * @param rules
   */
  addOrderRanking(rules: ISecuritySGRule[]) {
    const retRules = [];
    rules.forEach((rule, index) => {
      retRules.push(
        {
          order: index,
          rule: rule
        }
      );
    });
    return retRules;
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

}
