import { HttpClientTestingModule } from '@angular/common/http/testing';
import { DebugElement, Component, Directive, Input } from '@angular/core';
/**-----
 Angular imports
 ------------------*/
import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { MatIconRegistry } from '@angular/material';
import { By } from '@angular/platform-browser';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { RouterTestingModule } from '@angular/router/testing';
import { TestingUtility } from '@app/common/TestingUtility';
import { Utility } from '@app/common/Utility';
import { PrettyDatePipe } from '@app/components/shared/Pipes/PrettyDate.pipe';
import { SharedModule } from '@app/components/shared/shared.module';
/**-----
 Venice web-app imports
 ------------------*/
import { ControllerService } from '@app/services/controller.service';
import { SearchService } from '@app/services/generated/search.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { LogService } from '@app/services/logging/log.service';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { SearchPolicySearchRequest } from '@sdk/v1/models/generated/search';
import { SecurityService } from 'app/services/generated/security.service';
import { BehaviorSubject } from 'rxjs/BehaviorSubject';
import { SgpolicydetailComponent } from './sgpolicydetail.component';
import { ActivatedRoute } from '@angular/router';
import { Observable } from 'rxjs/Observable';
import { ReplaySubject } from 'rxjs/ReplaySubject';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { RouterLinkStubDirective } from '@app/common/RouterLinkStub.directive.spec';



function setNewRuleData(component, fixture, numRules: number = 40) {
  const tableRules = [];
  for (let index = 0; index < numRules; index++) {
    tableRules.push({
      order: index,
      rule: {
        'apps': [
          'tcp/8080',
        ],
        'action': 'PERMIT',
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


class MockActivatedRoute extends ActivatedRoute {
  policyId = 'policy1';
  paramObserver = new BehaviorSubject<any>({ id: this.policyId });
  snapshot: any = { url: ['security', 'sgpolicies', 'policy1'] };

  constructor() {
    super();
    this.params = this.paramObserver.asObservable();
  }

  setPolicyId(id) {
    this.policyId = id;
    this.paramObserver.next({ id: this.policyId });
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
        if (fieldIndex === 0) {
          // SG Policy doesn't have a header for the first column
          expect(field.nativeElement.textContent).toContain(rowIndex + 1);
          return;
        }
        const colData = columns[fieldIndex - 1];
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
            expect(field.nativeElement.textContent).toContain(rowData.action);
            break;
          case 'protocolPort':
            expect(field.nativeElement.textContent).toContain(rowData.apps.join(', '));
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
    expect(_.isEqual({ 'field-selector': 'ObjectMeta.Name=' + policyName }, calledObj)).toBeTruthy('Incorrect selector for ' + policyName);

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
      expect(fields[1].nativeElement.textContent.length).toBe(0);
    } else {
      expect(fields[1].nativeElement.textContent).toContain(formattedCreationTime);
    }
    // Mod time
    const formattedModTime = new PrettyDatePipe('en-US').transform(modTime);
    if (formattedModTime == null) {
      expect(fields[2].nativeElement.textContent.length).toBe(0);
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
    const inputs = fixture.debugElement.queryAll(By.css('mat-form-field'));
    return inputs[0].query(By.css('input'));
  }

  function getDestIpInput() {
    const inputs = fixture.debugElement.queryAll(By.css('mat-form-field'));
    return inputs[1].query(By.css('input'));
  }

  function getPortInput() {
    const inputs = fixture.debugElement.queryAll(By.css('mat-form-field'));
    return inputs[2].query(By.css('input'));
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

  beforeEach(async(() => {
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
        LogService,
        LogPublishersService,
        MatIconRegistry,
        SearchService,
        SecurityService,
        UIConfigsService,
        {
          provide: ActivatedRoute,
          useClass: MockActivatedRoute
        }
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(SgpolicydetailComponent);
    component = fixture.componentInstance;
    testingUtility = new TestingUtility(fixture);
    component.cols = [
      { field: 'meta', header: 'Meta' },
      { field: 'spec', header: 'Spec' },
      { field: 'status', header: 'Status' }
    ];
    const securityService = TestBed.get(SecurityService);
    const rules1 = [
      {
        'apps': [
          'tcp/80',
          'udp/53'
        ],
        'action': 'PERMIT',
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
        'action': 'DENY',
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
        'action': 'PERMIT',
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
        'action': 'DENY',
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
      body: {
        result: {
          Events: [
            {
              Type: 'Created',
              Object: sgPolicy1
            }
          ]
        }
      }
    });
    sgPolicyWatchSpy = spyOn(securityService, 'WatchSGPolicy').and.returnValue(
      sgPolicyObserver
    );
    sgPolicyGetSpy = spyOn(securityService, 'GetSGPolicy').and.returnValue(
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
    expect(_.isEqual(expectedReq.getValues(), calledObj.getValues())).toBeTruthy();

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
    expect(_.isEqual(expectedReq.getValues(), calledObj.getValues())).toBeTruthy();

    // Putting text into APP
    testingUtility.setText(portInput, 'tcp/80');
    testingUtility.sendEnterKeyup(sourceIPInput);
    expectedReq = new SearchPolicySearchRequest();
    expectedReq['from-ip-address'] = '10.1.1.1';
    expectedReq['tenant'] = 'default';
    expectedReq['namespace'] = 'default';
    expectedReq['to-ip-address'] = '10.1.1.8';
    expectedReq['app'] = 'tcp/80';
    expectedReq['sg-policy'] = 'policy1';
    calledObj = postQuerySpy.calls.mostRecent().args[0];
    expect(_.isEqual(expectedReq.getValues(), calledObj.getValues())).toBeTruthy();

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

    // Should allow port only search
    testingUtility.setText(sourceIPInput, '');
    testingUtility.setText(portInput, 'tcp/88');
    searchButton = getSearchButton();
    expect(searchButton).toBeTruthy();

    // Click the search button should invoke a search
    testingUtility.sendClick(searchButton);
    expect(service.PostPolicyQuery).toHaveBeenCalledTimes(1);
    const req = querySpy.calls.first().args[0];
    expect(req['from-ip-address']).toBe('any');
    expect(req['to-ip-address']).toBe('any');
    expect(req.app).toBe('tcp/88');
  });


  it('should display sgpolicy meta/rules and highlight matching row or display there is none on search', () => {
    fixture.detectChanges();
    verifyMeta(sgPolicy1.meta.name, sgPolicy1.meta['creation-time'], sgPolicy1.meta['mod-time']);
    verifyTable(sgPolicy1.spec.rules, component.cols);


    // Mocking enough entries to test search highlighting
    setNewRuleData(component, fixture, 40);
    const searchService = TestBed.get(SearchService);
    let matchingIndex = 10;
    const postQuerySpy = spyOn(searchService, 'PostPolicyQuery').and.returnValue(
      new BehaviorSubject<any>({
        body: {
          status: 'MATCH',
          results: {
            'policy1': {
              'index': matchingIndex
            }
          }
        }
      }
      ));
    spyOn(component.lazyRenderWrapper, 'scrollToRowNumber');
    component.invokePolicySearch('10.1.1.1');
    fixture.detectChanges();
    expect(searchService.PostPolicyQuery).toHaveBeenCalled();
    // 10th row should be highlighted and we should have scrolled to it
    expect(component.selectedRuleIndex).toBe(10);
    let rule = getMatchingRule();
    expect(rule).toBeTruthy();
    expect(rule.children[0].nativeElement.textContent).toContain(matchingIndex + 1);
    expect(component.lazyRenderWrapper.scrollToRowNumber).toHaveBeenCalledWith(matchingIndex);
    // If user starts to modify search, the highlight row disappears
    const sourceIPInput = getSourceIpInput();
    testingUtility.setText(sourceIPInput, '192');
    // Listens for key up, so we trigger one with a random keyCode
    sourceIPInput.triggerEventHandler('keyup', { keyCode: 20 });
    fixture.detectChanges();
    rule = getMatchingRule();
    expect(rule).toBeNull();

    // Search with a miss or missing policy name
    postQuerySpy.and.returnValue(
      new BehaviorSubject<any>({
        body: {
          status: 'MISS'
        }
      }
      ));
    component.invokePolicySearch('10.1.1.1');
    fixture.detectChanges();
    let errorMessageDiv = getSearchErrorDiv();
    expect(errorMessageDiv).toBeTruthy();
    expect(errorMessageDiv.children[1].nativeElement.textContent).toContain('No Matching Rule');

    // Checking results from a different policy don't register a match
    postQuerySpy.and.returnValue(
      new BehaviorSubject<any>({
        body: {
          status: 'MATCH',
          results: {
            'randomPolicy': {
              'index': 10
            }
          }
        }
      }
      ));
    component.invokePolicySearch('10.1.1.1');
    fixture.detectChanges();
    errorMessageDiv = getSearchErrorDiv();
    expect(errorMessageDiv).toBeTruthy();
    expect(errorMessageDiv.children[1].nativeElement.textContent).toContain('No Matching Rule');

    // Setting a valid search for the following test examples
    postQuerySpy.and.returnValue(
      new BehaviorSubject<any>({
        body: {
          status: 'MATCH',
          results: {
            'policy1': {
              'index': 10
            }
          }
        }
      }
      ));
    component.invokePolicySearch('10.1.1.1');
    fixture.detectChanges();
    errorMessageDiv = getSearchErrorDiv();
    expect(errorMessageDiv).toBeNull();

    // Causing new data button
    component.lazyRenderWrapper.hasUpdate = true;

    // Setting up for the next search
    postQuerySpy.calls.reset();
    matchingIndex = 5;
    postQuerySpy.and.returnValue(
      new BehaviorSubject<any>({
        body: {
          status: 'MATCH',
          results: {
            'policy1': {
              'index': matchingIndex
            }
          }
        }
      }
      ));

    // if user clicks new data button, we should redo the search
    component.lazyRenderWrapper.resetTableView(); // Equivalent of clicking update data button
    fixture.detectChanges();
    expect(searchService.PostPolicyQuery).toHaveBeenCalled();
    // 5th row should be highlighted and we should have scrolled to it
    expect(component.selectedRuleIndex).toBe(matchingIndex);
    rule = getMatchingRule();
    expect(rule).toBeTruthy();
    expect(rule.children[0].nativeElement.textContent).toContain(matchingIndex + 1);
    expect(component.lazyRenderWrapper.scrollToRowNumber).toHaveBeenCalledWith(matchingIndex);

    // If there is new data, and user tries to perform a search
    // we should force an update to the new data
    component.lazyRenderWrapper.hasUpdate = true;
    component.invokePolicySearch('10.1.1.1');
    fixture.detectChanges();
    // Should have switched to new data and invoked search
    expect(component.lazyRenderWrapper.hasUpdate).toBeFalsy();
    expect(searchService.PostPolicyQuery).toHaveBeenCalledTimes(2);
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
    expect(buttons[0].nativeElement.textContent).toContain('SG POLICIES');
    expect(buttons[1].nativeElement.textContent).toContain('HOMEPAGE');
    // Form controls should be disabled
    expect(getSourceIpInput().nativeElement.disabled).toBeTruthy();
    expect(getDestIpInput().nativeElement.disabled).toBeTruthy();
    expect(getPortInput().nativeElement.disabled).toBeTruthy();

    // Add object
    policyWatchObserver.next({
      body: {
        result: {
          Events: [
            {
              Type: 'Created',
              Object: sgPolicy2
            }
          ]
        }
      }
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
      body: {
        result: {
          Events: [
            {
              Type: 'Deleted',
              Object: sgPolicy2
            }
          ]
        }
      }
    });

    fixture.detectChanges();

    expect(getOverlay()).toBeTruthy();
    expect(getDeletedPolicyIcon()).toBeTruthy();
    expect(getOverlayText().nativeElement.textContent).toContain('policy2 has been deleted');
    buttons = getOverlayButtons();
    expect(buttons.length).toBe(2);
    expect(buttons[0].nativeElement.textContent).toContain('SG POLICIES');
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
        body: {
          result: {
            Events: [
              {
                Type: 'Created',
                Object: sgPolicy2
              }
            ]
          }
        }
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

});
