import { HttpClientTestingModule } from '@angular/common/http/testing';
import { DebugElement, Component, Directive, Input, NO_ERRORS_SCHEMA } from '@angular/core';
/**-----
 Angular imports
 ------------------*/
import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { MatIconRegistry } from '@angular/material';
import { By } from '@angular/platform-browser';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { RouterTestingModule } from '@angular/router/testing';
import { TestingUtility } from '@app/common/TestingUtility';
import { Utility } from '@app/common/Utility';
import { PrettyDatePipe } from '@app/components/shared/Pipes/PrettyDate.pipe';
import { SharedModule } from '@app/components/shared/shared.module';
import { SecuritySGRule_action_uihint } from '@sdk/v1/models/generated/security';
/**-----
 Venice web-app imports
 ------------------*/
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { SearchService } from '@app/services/generated/search.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { LogService } from '@app/services/logging/log.service';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { SearchPolicySearchRequest } from '@sdk/v1/models/generated/search';
import { SecurityService } from 'app/services/generated/security.service';
import { BehaviorSubject, Observable, ReplaySubject } from 'rxjs';
import { SgpolicydetailComponent } from './sgpolicydetail.component';
import { ActivatedRoute } from '@angular/router';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { RouterLinkStubDirective } from '@app/common/RouterLinkStub.directive.spec';
import { MessageService } from '@app/services/message.service';
import { AuthService } from '@app/services/auth.service';
import { MetricsqueryService } from '@app/services/metricsquery.service';
import { ITelemetry_queryMetricsQueryResponse } from '@sdk/v1/models/telemetry_query';
import { TelemetryqueryService } from '@app/services/generated/telemetryquery.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { WorkloadService } from '@app/services/generated/workload.service';




function setNewRuleData(component, fixture, numRules: number = 40) {
  const tableRules = [];
  for (let index = 0; index < numRules; index++) {
    tableRules.push({
      order: index,
      rule: {
        'apps': [
          'customApp',
        ],
        'proto-ports': [
          {
            'protocol': 'tcp',
            'ports': '8080'
          },
          {
            'protocol': 'udp',
            'ports': '53'
          },
        ],
        'action': 'permit',
        'from-ip-addresses': [
          '10.1.1.0'
        ],
        'to-ip-addresses': [
          '10.1.1.8'
        ]
      }
    });
  }
  component.sgPolicyRules = tableRules;
  fixture.detectChanges();
}

function resetSearchInputs(src: DebugElement, dest: DebugElement, app: DebugElement, testingUtility: TestingUtility) {
  testingUtility.setText(src, '');
  testingUtility.setText(dest, '');
  testingUtility.setText(app, '');
}

class MockActivatedRoute extends ActivatedRoute {
  policyId = 'policy1';
  paramObserver = new BehaviorSubject<any>({id: this.policyId});
  snapshot: any = {url: ['security', 'sgpolicies', 'policy1']};

  constructor() {
    super();
    this.params = this.paramObserver.asObservable();
  }

  setPolicyId(id) {
    this.policyId = id;
    this.paramObserver.next({id: this.policyId});
  }
}

describe('SgpolicydetailComponent', () => {
  const _ = Utility.getLodash();

  let component: SgpolicydetailComponent;
  let fixture: ComponentFixture<SgpolicydetailComponent>;
  let testingUtility: TestingUtility;
  let sgPolicyWatchSpy;
  let sgPolicyGetSpy;
  let sgPolicyObserver;
  let sgPolicy1;
  let sgPolicy2;

  /**
   * We don't use the verify table in TestingUtility as we need to skip the
   * first field index
   */
  function verifyTable(data: any[], columns: any[]) {
    const tableElem = fixture.debugElement.query(By.css('.ui-table-scrollable-body-table tbody'));
    const rows = tableElem.queryAll(By.css('tr'));
    expect(rows.length).toBe(data.length, 'Data did not match number of entries in the table');
    rows.forEach((row, rowIndex) => {
      const rowData = data[rowIndex];
      row.children.forEach((field, fieldIndex) => {
        if (fieldIndex === 0 ) {
          return 0;
        }
        if (fieldIndex === 1) {
          // SG Policy doesn't have a header for the first column
          expect(field.nativeElement.textContent).toContain(rowIndex + 1);
          return;
        }
        const colData = columns[fieldIndex - 1];
        if (colData === undefined) {
          return 0;
        }
        switch (colData.field) {
          case 'sourceIPs':
            expect(field.nativeElement.textContent)
              .toContain(rowData['from-ip-addresses'].join(', '),
                'source IPs time did not match for row ' + rowIndex);
            break;

          case 'destIPs':
            expect(field.nativeElement.textContent)
              .toContain(rowData['to-ip-addresses'].join(', '),
                'dest IPs time did not match for row ' + rowIndex);
            break;

          case 'action':
            expect(field.nativeElement.textContent).toContain(SecuritySGRule_action_uihint[rowData.action]);
            break;
          case 'protocolPort':
            expect(field.nativeElement.textContent).toContain(rowData.apps.join(', '));
            if (rowData['proto-ports'] != null) {
              rowData['proto-ports'].forEach((entry) => {
                expect(field.nativeElement.textContent).toContain(entry.protocol + '/' + entry.ports);
              });
            }

            break;
          case 'TotalHits':
            expect(field.nativeElement.textContent).toContain('');
            break;
          default:
            const fieldData = Utility.getObjectValueByPropertyPath(data[rowIndex], colData.field.split('.'));
            expect(field.nativeElement.textContent).toContain(fieldData, colData.header + ' did not match');
        }
      });
    });
  }

  function verifyServiceCalls(policyName) {
    expect(sgPolicyWatchSpy).toHaveBeenCalled();
    const calledObj = sgPolicyWatchSpy.calls.mostRecent().args[0];
    expect(_.isEqual({'field-selector': 'meta.name=' + policyName}, calledObj)).toBeTruthy('Incorrect selector for ' + policyName);

    expect(sgPolicyGetSpy).toHaveBeenCalled();
    expect(sgPolicyGetSpy).toHaveBeenCalledWith(policyName);
  }

  function verifyMeta(name, creationTime, modTime) {
    const fields = fixture.debugElement.queryAll(By.css('.sgpolicy-summary-panel-content-value'));
    expect(fields.length).toBe(3);
    expect(fields[0].nativeElement.textContent).toContain(name);
    // Creation time
    const formattedCreationTime = new PrettyDatePipe('en-US').transform(creationTime);
    if (formattedCreationTime == null) {
      expect(fields[1].nativeElement.textContent.trim().length).toBe(0);
    } else {
      expect(fields[1].nativeElement.textContent).toContain(formattedCreationTime);
    }
    // Mod time
    const formattedModTime = new PrettyDatePipe('en-US').transform(modTime);
    if (formattedModTime == null) {
      expect(fields[2].nativeElement.textContent.trim().length).toBe(0);
    } else {
      expect(fields[2].nativeElement.textContent).toContain(formattedModTime);
    }
  }

  function getSearchErrorDiv() {
    return fixture.debugElement.query(By.css('.sgpolicy-search-error'));
  }

  function getSearchButton() {
    return fixture.debugElement.query(By.css('.sgpolicy-search-button'));
  }

  function getSearchClearButton() {
    return fixture.debugElement.query(By.css('.sgpolicy-search-clear-button'));
  }

  function getSourceIpInput() {
    const inputs = fixture.debugElement.queryAll(By.css('.sgpolicydetail-inputip'));
    return inputs[0];
  }

  function getDestIpInput() {
    const inputs = fixture.debugElement.queryAll(By.css('.sgpolicydetail-inputip'));
    return inputs[1];
  }

  function getPortInput() {
    const inputs = fixture.debugElement.queryAll(By.css('.sgpolicydetail-inputip'));
    return inputs[2];
  }

  function getOverlay() {
    return fixture.debugElement.query(By.css('.sgpolicy-overlay'));
  }

  function getMatchingRule() {
    return fixture.debugElement.query(By.css('.sgpolicy-match'));
  }

  function getMissingPolicyIcon() {
    return fixture.debugElement.query(By.css('.sgpolicy-missing-policy'));
  }

  function getDeletedPolicyIcon() {
    return fixture.debugElement.query(By.css('.sgpolicy-deleted-policy'));
  }

  function getOverlayText() {
    return fixture.debugElement.query(By.css('.sgpolicy-overlay-text'));
  }

  function getOverlayButtons() {
    return fixture.debugElement.queryAll(By.css('.sgpolicy-overlay-button'));
  }

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [SgpolicydetailComponent, RouterLinkStubDirective],
      imports: [
        RouterTestingModule,
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
        SharedModule,
        HttpClientTestingModule,
        PrimengModule,
        MaterialdesignModule
      ],
      providers: [
        ControllerService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        SearchService,
        SecurityService,
        UIConfigsService,
        AuthService,
        MessageService,
        ClusterService,
        MetricsqueryService,
        WorkloadService,
        {
          provide: ActivatedRoute,
          useClass: MockActivatedRoute
        }
      ],
      schemas: [NO_ERRORS_SCHEMA],
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(SgpolicydetailComponent);
    component = fixture.componentInstance;
    testingUtility = new TestingUtility(fixture);
    const securityService = TestBed.get(SecurityService);
    const rules1 = [
      {
        'apps': [
          'tcp/80',
          'udp/53'
        ],
        'action': 'permit',
        'from-ip-addresses': [
          '172.0.0.1'
        ],
        'to-ip-addresses': [
          '192.168.1.1/16'
        ]
      },
      {
        'apps': [
          'tcp/84',
        ],
        'action': 'deny',
        'from-ip-addresses': [
          '10.1.1.2'
        ],
        'to-ip-addresses': [
          '10.1.1.3'
        ]
      },
      {
        'apps': [
          'tcp/8080',
        ],
        'action': 'permit',
        'from-ip-addresses': [
          '10.1.1.0'
        ],
        'to-ip-addresses': [
          '10.1.1.8'
        ]
      }
    ];

    const rules2 = [
      {
        'apps': [
          'tcp/84',
        ],
        'action': 'deny',
        'from-ip-addresses': [
          '10.1.1.2'
        ],
        'to-ip-addresses': [
          '10.1.1.3'
        ]
      }
    ];

    sgPolicy1 = {
      meta: {
        name: 'policy1',
        'mod-time': '2018-08-23T17:35:08.534909931Z',
        'creation-time': '2018-08-23T17:30:08.534909931Z'
      },
      spec: {
        'rules': rules1
      },
      status: {
        'rule-status': [
          'hash1'
        ]
      }
    };

    sgPolicy2 = {
      meta: {
        name: 'policy2',
        'mod-time': '2018-08-23T17:35:08.534909931Z',
        'creation-time': '2018-08-23T17:30:08.534909931Z'
      },
      spec: {
        'rules': rules2
      }
    };

    sgPolicyObserver = new BehaviorSubject({
      events: [
        {
          type: 'Created',
          object: sgPolicy1
        }
      ]
    });
    sgPolicyWatchSpy = spyOn(securityService, 'WatchNetworkSecurityPolicy').and.returnValue(
      sgPolicyObserver
    );
    sgPolicyGetSpy = spyOn(securityService, 'GetNetworkSecurityPolicy').and.returnValue(
      new BehaviorSubject({
        body: sgPolicy1
      })
    );
  });

  afterEach(() => {
    TestBed.resetTestingModule();
  });

  it('should use sgpolicy name, source/dest IP, and port fields when making query', () => {
    fixture.detectChanges();
    component.selectedPolicyId = 'policy1';

    const service = TestBed.get(SearchService);
    const postQuerySpy = spyOn(service, 'PostPolicyQuery').and.returnValue(
      new BehaviorSubject<any>(
        {
          body: {
            status: 'MISS'
          }
        }
      ));

    const sourceIPInput = getSourceIpInput();
    const destIPInput = getDestIpInput();
    const portInput = getPortInput();

    // Putting in text into source IP
    testingUtility.setText(sourceIPInput, '10.1.1.1');
    testingUtility.sendEnterKeyup(sourceIPInput);
    let expectedReq = new SearchPolicySearchRequest();
    expectedReq['to-ip-address'] = 'any';
    expectedReq['from-ip-address'] = '10.1.1.1';
    expectedReq['tenant'] = 'default';
    expectedReq['namespace'] = 'default';
    expectedReq['sg-policy'] = 'policy1';
    let calledObj = postQuerySpy.calls.mostRecent().args[0];
    expect(_.isEqual(expectedReq.getFormGroupValues(), calledObj.getFormGroupValues())).toBeTruthy();

    // Putting in text into dest IP
    testingUtility.setText(destIPInput, '10.1.1.8');
    testingUtility.sendEnterKeyup(sourceIPInput);
    expectedReq = new SearchPolicySearchRequest();
    expectedReq['from-ip-address'] = '10.1.1.1';
    expectedReq['tenant'] = 'default';
    expectedReq['namespace'] = 'default';
    expectedReq['to-ip-address'] = '10.1.1.8';
    expectedReq['sg-policy'] = 'policy1';
    calledObj = postQuerySpy.calls.mostRecent().args[0];
    expect(_.isEqual(expectedReq.getFormGroupValues(), calledObj.getFormGroupValues())).toBeTruthy();

    // Putting text into APP
    testingUtility.setText(portInput, 'tcp/80');
    testingUtility.sendEnterKeyup(sourceIPInput);
    expectedReq = new SearchPolicySearchRequest();
    expectedReq['from-ip-address'] = '10.1.1.1';
    expectedReq['tenant'] = 'default';
    expectedReq['namespace'] = 'default';
    expectedReq['to-ip-address'] = '10.1.1.8';
    expectedReq['protocol'] = 'tcp';
    expectedReq['port'] = '80';
    expectedReq['sg-policy'] = 'policy1';
    calledObj = postQuerySpy.calls.mostRecent().args[0];
    expect(_.isEqual(expectedReq.getFormGroupValues(), calledObj.getFormGroupValues())).toBeTruthy();

    // Putting text into APP
    testingUtility.setText(portInput, 'appSearch');
    testingUtility.sendEnterKeyup(sourceIPInput);
    expectedReq = new SearchPolicySearchRequest();
    expectedReq['from-ip-address'] = '10.1.1.1';
    expectedReq['tenant'] = 'default';
    expectedReq['namespace'] = 'default';
    expectedReq['to-ip-address'] = '10.1.1.8';
    expectedReq['app'] = 'appSearch';
    expectedReq['sg-policy'] = 'policy1';
    calledObj = postQuerySpy.calls.mostRecent().args[0];
    expect(_.isEqual(expectedReq.getFormGroupValues(), calledObj.getFormGroupValues())).toBeTruthy();

  });

  it('should reset table contents if search input are all empty', () => {
    fixture.detectChanges();
    const updateRulesByPolicySpy = spyOn(component, 'updateRulesByPolicy').and.callThrough();
    const sourceIPInput = getSourceIpInput();
    const destIPInput = getDestIpInput();
    const portInput = getPortInput();
    resetSearchInputs(sourceIPInput, destIPInput, portInput, testingUtility);
    testingUtility.sendEnterKeyup(sourceIPInput);

    expect(component.updateRulesByPolicy).toHaveBeenCalled();
  });

  it('should reset table contents when click clear search button', () => {
    fixture.detectChanges();
    const updateRulesByPolicySpy = spyOn(component, 'updateRulesByPolicy').and.callThrough();
    const sourceIPInput = getSourceIpInput();
    const destIPInput = getDestIpInput();
    const portInput = getPortInput();

    testingUtility.setText(sourceIPInput, '192');
    const searchButton = getSearchButton();
    const searchClearButton = getSearchClearButton();
    testingUtility.sendClick(searchClearButton);

    expect(component.updateRulesByPolicy).toHaveBeenCalled();
  });

  it('should show search/cancel buttons when either src ip, dest ip or app is not empty', () => {
    fixture.detectChanges();
    const invokePolicySearchSpy = spyOn(component, 'invokePolicySearch').and.callThrough();
    const service = TestBed.get(SearchService);
    const querySpy = spyOn(service, 'PostPolicyQuery').and.returnValue(
      new BehaviorSubject<any>({
          body: {
            status: 'MISS'
          }
        }
      ));

    // No inputs have text, so search and clear buttons are undefined
    // There should be no error message
    let searchButton = getSearchButton();
    let searchClearButton = getSearchClearButton();
    const errorMessageDiv = getSearchErrorDiv();
    expect(searchButton).toBeNull();
    expect(searchClearButton).toBeNull();
    expect(errorMessageDiv).toBeNull();

    // search inputs
    const sourceIPInput = getSourceIpInput();
    const destIPInput = getDestIpInput();
    const portInput = getPortInput();

    // ---------------------------
    // Putting in text into source IP
    testingUtility.setText(sourceIPInput, '192');

    // The search and cancel buttons should appear
    searchButton = getSearchButton();
    searchClearButton = getSearchClearButton();
    expect(searchButton).toBeTruthy();
    expect(searchClearButton).toBeTruthy();

    resetSearchInputs(sourceIPInput, destIPInput, portInput, testingUtility);

    // ---------------------------
    // Putting in text into dest IP
    testingUtility.setText(destIPInput, '192');

    // The search and cancel buttons should appear
    searchButton = getSearchButton();
    searchClearButton = getSearchClearButton();
    expect(searchButton).toBeTruthy();
    expect(searchClearButton).toBeTruthy();

    resetSearchInputs(sourceIPInput, destIPInput, portInput, testingUtility);

    // ---------------------------
    // Putting in text into port
    testingUtility.setText(portInput, '80');

    // The search and cancel buttons should appear
    searchButton = getSearchButton();
    searchClearButton = getSearchClearButton();
    expect(searchButton).toBeTruthy();
    expect(searchClearButton).toBeTruthy();

    resetSearchInputs(sourceIPInput, destIPInput, portInput, testingUtility);

  });

  it('should show search/cancel buttons when there is input and error message when input searched is not a valid IP', () => {
    fixture.detectChanges();

    const invokePolicySearchSpy = spyOn(component, 'invokePolicySearch').and.callThrough();
    const service = TestBed.get(SearchService);
    const querySpy = spyOn(service, 'PostPolicyQuery').and.returnValue(
      new BehaviorSubject<any>({
          body: {
            status: 'MISS'
          }
        }
      ));

    // No inputs have text, so search and clear buttons are undefined
    // There should be no error message
    let searchButton = getSearchButton();
    let searchClearButton = getSearchClearButton();
    let errorMessageDiv = getSearchErrorDiv();
    expect(searchButton).toBeNull();
    expect(searchClearButton).toBeNull();
    expect(errorMessageDiv).toBeNull();

    // Putting in text into source IP
    const sourceIPInput = getSourceIpInput();
    const portInput = getPortInput();
    testingUtility.setText(sourceIPInput, '192');
    // The search and cancel buttons should appear
    searchButton = getSearchButton();
    searchClearButton = getSearchClearButton();
    expect(searchButton).toBeTruthy();
    expect(searchClearButton).toBeTruthy();
    // Putting in text into dest IP
    const destIPInput = getDestIpInput();
    testingUtility.setText(destIPInput, '10.1.1.1');
    // The search and cancel buttons should appear
    searchButton = getSearchButton();
    searchClearButton = getSearchClearButton();
    expect(searchButton).toBeTruthy();
    expect(searchClearButton).toBeTruthy();
    // Click the search button should invoke a search
    testingUtility.sendClick(searchButton);
    expect(component.invokePolicySearch).toHaveBeenCalled();
    expect(service.PostPolicyQuery).toHaveBeenCalledTimes(0);
    // There should be an invalid IP message
    errorMessageDiv = getSearchErrorDiv();
    expect(errorMessageDiv).toBeTruthy();
    expect(errorMessageDiv.children[1].nativeElement.textContent).toContain('Invalid IP');
    // Typing again should remove the message if the content is different
    testingUtility.setText(sourceIPInput, '192');
    errorMessageDiv = getSearchErrorDiv();
    expect(errorMessageDiv).toBeTruthy();
    testingUtility.setText(sourceIPInput, '192.10');
    // Listens for key up, so we trigger one with a random keyCode
    sourceIPInput.triggerEventHandler('keyup', { keyCode: 20 });
    fixture.detectChanges();
    errorMessageDiv = getSearchErrorDiv();
    expect(errorMessageDiv).toBeNull();
    // Clicking clear button should empty out the results, but not reset the scroll
    // since we don't have a match
    // TODO: Find a way to check scroll
    testingUtility.sendClick(searchClearButton);
    expect(sourceIPInput.nativeElement.value).toBe('');

    // allow port only search
    testingUtility.setText(sourceIPInput, '');
    testingUtility.setText(portInput, 'tcp/88');
    searchButton = getSearchButton();
    expect(searchButton).toBeTruthy();
  });

  it('should display missing policy overlay and deleted policy overlay', () => {
    // change param id
    const mockActivatedRoute: MockActivatedRoute = TestBed.get(ActivatedRoute);
    mockActivatedRoute.setPolicyId('policy2');
    const policyWatchObserver = new ReplaySubject();
    const policyGetObserver = new Observable((observable) => {
      observable.error({ body: null, statusCode: 400 });
    });
    sgPolicyWatchSpy.and.returnValue(
      policyWatchObserver
    );
    sgPolicyGetSpy.and.returnValue(
      policyGetObserver
    );

    fixture.detectChanges();
    verifyMeta('', '', '');
    verifyServiceCalls('policy2');

    // View should now be of missing overlay, and data should be cleared
    verifyTable([], component.cols);
    expect(getOverlay()).toBeTruthy();
    expect(getMissingPolicyIcon()).toBeTruthy();
    expect(getOverlayText().nativeElement.textContent).toContain('policy2 does not exist');
    let buttons = getOverlayButtons();
    expect(buttons.length).toBe(2);
    expect(buttons[0].nativeElement.textContent).toContain('POLICIES');
    expect(buttons[1].nativeElement.textContent).toContain('HOMEPAGE');
    // Form controls should be disabled
    expect(getSourceIpInput().nativeElement.disabled).toBeTruthy();
    expect(getDestIpInput().nativeElement.disabled).toBeTruthy();
    expect(getPortInput().nativeElement.disabled).toBeTruthy();

    // Add object
    policyWatchObserver.next({
      events: [
        {
          type: 'Created',
          object: sgPolicy2
        }
      ]
    });

    fixture.detectChanges();
    // overlay should be gone
    expect(getOverlay()).toBeNull();
    verifyMeta(sgPolicy2.meta.name, sgPolicy2.meta['creation-time'], sgPolicy2.meta['mod-time']);
    verifyTable(sgPolicy2.spec.rules, component.cols);
    // Form controls should be enabled
    expect(getSourceIpInput().nativeElement.disabled).toBeFalsy();
    expect(getDestIpInput().nativeElement.disabled).toBeFalsy();
    expect(getPortInput().nativeElement.disabled).toBeFalsy();

    // Delete policy
    policyWatchObserver.next({
      events: [
        {
          type: 'Deleted',
          object: sgPolicy2
        }
      ]
    });

    fixture.detectChanges();

    expect(getOverlay()).toBeTruthy();
    expect(getDeletedPolicyIcon()).toBeTruthy();
    expect(getOverlayText().nativeElement.textContent).toContain('policy2 has been deleted');
    buttons = getOverlayButtons();
    expect(buttons.length).toBe(2);
    expect(buttons[0].nativeElement.textContent).toContain('POLICIES');
    expect(buttons[1].nativeElement.textContent).toContain('HOMEPAGE');
    // Form controls should be disabled
    expect(getSourceIpInput().nativeElement.disabled).toBeTruthy();
    expect(getDestIpInput().nativeElement.disabled).toBeTruthy();
    expect(getPortInput().nativeElement.disabled).toBeTruthy();

    // Clicking homepage button
    spyOn(component, 'routeToHomepage');
    testingUtility.sendClick(buttons[1]);
    expect(component.routeToHomepage).toHaveBeenCalled();

    // find DebugElements with an attached RouterLinkStubDirective
    const linkDes = fixture.debugElement
      .queryAll(By.directive(RouterLinkStubDirective));

    // get attached link directive instances
    // using each DebugElement's injector
    const routerLinks = linkDes.map(de => de.injector.get(RouterLinkStubDirective));
    expect(routerLinks.length).toBe(1, 'should have 1 routerLinks');
    expect(routerLinks[0].linkParams).toBe('../');

    testingUtility.sendClick(buttons[0]);
    expect(routerLinks[0].navigatedTo).toBe('../');

  });


  it('should rerender when user navigates to same page with different id and use field selectors', () => {
    fixture.detectChanges();
    verifyMeta(sgPolicy1.meta.name, sgPolicy1.meta['creation-time'], sgPolicy1.meta['mod-time']);
    verifyServiceCalls('policy1');

    // change param id
    let mockActivatedRoute: MockActivatedRoute = TestBed.get(ActivatedRoute);
    sgPolicyWatchSpy.and.returnValue(
      new BehaviorSubject({
        events: [
          {
            type: 'Created',
            object: sgPolicy2
          }
        ]
      })
    );
    mockActivatedRoute.setPolicyId('policy2');


    // View should now be of policy2
    fixture.detectChanges();
    verifyMeta(sgPolicy2.meta.name, sgPolicy2.meta['creation-time'], sgPolicy2.meta['mod-time']);
    verifyServiceCalls('policy2');
    verifyTable(sgPolicy2.spec.rules, component.cols);

    // change param id
    const policyWatchObserver = new ReplaySubject();
    const policyGetObserver = new Observable((observable) => {
      observable.error({ body: null, statusCode: 400 });
    });
    sgPolicyWatchSpy.and.returnValue(
      policyWatchObserver
    );
    sgPolicyGetSpy.and.returnValue(
      policyGetObserver
    );
    mockActivatedRoute = TestBed.get(ActivatedRoute);
    mockActivatedRoute.setPolicyId('policy3');

    verifyServiceCalls('policy3');

    // View should now be of missing overlay, and data should be cleared
    fixture.detectChanges();
    verifyMeta('', '', '');
    verifyTable([], component.cols);
    expect(getOverlay()).toBeTruthy();

  });

  it('should fetch rule metrics', () => {
    const data: ITelemetry_queryMetricsQueryResponse = {
      results: [
        {
          series: [
            {
              tags: {
                name: 'hash1'
              },
              columns: [
                'time',
                'EspHits',
                'IcmpHits',
                'OtherHits',
                'TcpHits',
                'TotalHits',
                'UdpHits',
                'reporterID',
                'namespace',
                'tenant'
              ],
              values: [
                ['', 0, 0, 0, 0, 0, 15, 'naples1', '', ''],
                ['', 2, 0, 0, 0, 0, 2, 'naples2', '', ''],
                ['', 0, 2, 0, 0, 0, 2, 'naples3', '', ''],
                ['', 0, 0, 0, 0, 0, 10, 'naples1', '', ''],
              ]
            }
          ]
        }
      ]
    };
    const service = TestBed.get(MetricsqueryService);
    const spy = spyOn(service, 'pollMetrics').and.returnValue(
      new BehaviorSubject<any>(data)
    );
    fixture.detectChanges();
    expect(spy).toHaveBeenCalled();
    expect(component.ruleMetrics['hash1']).toBeTruthy();
    expect(component.ruleMetrics['hash1']['UdpHits']).toBe(19);
    expect(component.ruleMetrics['hash1']['EspHits']).toBe(2);
    expect(component.ruleMetrics['hash1']['IcmpHits']).toBe(2);
  });


});
