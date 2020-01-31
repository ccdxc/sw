import { ComponentFixture, TestBed } from '@angular/core/testing';

import { AdvancedSearchComponent } from './advanced-search.component';
import {WidgetsModule} from 'web-app-framework';
import {FormsModule, ReactiveFormsModule} from '@angular/forms';
import {NoopAnimationsModule} from '@angular/platform-browser/animations';
import {CommonModule} from '@angular/common';
import {FlexLayoutModule} from '@angular/flex-layout';
import {MaterialdesignModule} from '@lib/materialdesign.module';
import {PrimengModule} from '@lib/primeng.module';
import {ControllerService} from '@app/services/controller.service';
import {RouterTestingModule} from '@angular/router/testing';
import {HttpClientTestingModule} from '@angular/common/http/testing';
import {LogService} from '@app/services/logging/log.service';
import {ConfirmationService} from 'primeng/api';
import {LogPublishersService} from '@app/services/logging/log-publishers.service';
import {ClusterService} from '@app/services/generated/cluster.service';
import {MatIconRegistry} from '@angular/material';
import {MetricsqueryService} from '@app/services/metricsquery.service';
import {MessageService} from '@app/services/message.service';
import {By} from '@angular/platform-browser';
import {TestingUtility} from '@common/TestingUtility';
import { configureTestSuite } from 'ng-bullet';
import { NaplesConditionValues } from '@app/components/cluster-group/naples';
import {ValueType} from 'web-app-framework';
import { SearchUtil } from '@components/search/SearchUtil';

describe('AdvancedSearchComponent', () => {
  let component: AdvancedSearchComponent;
  let fixture: ComponentFixture<AdvancedSearchComponent>;
  let tu: TestingUtility;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [ AdvancedSearchComponent ],
      imports: [
        CommonModule,
        NoopAnimationsModule,
        FlexLayoutModule,
        MaterialdesignModule,
        PrimengModule,
        FormsModule,
        ReactiveFormsModule,
        FormsModule,
        WidgetsModule,
        RouterTestingModule,
        HttpClientTestingModule
      ],
      providers: [
        ControllerService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        ClusterService,
        MatIconRegistry,
        MetricsqueryService,
        MessageService
      ]
    });
  });

  function setupAuditEventInput(c: AdvancedSearchComponent) {
    c.cols = [
      { field: 'user.name', header: 'Who', class: 'auditevents-column-common auditevents-column-who', sortable: false, width: 10, kind: 'AuditEvent' },
      { field: 'meta.mod-time', header: 'Time', class: 'auditevents-column-common auditevents-column-date', sortable: true, width: 9, kind: 'AuditEvent' },
      { field: 'action', header: 'Action', class: 'auditevents-column-common auditevents-column-action', sortable: false, width: 9, kind: 'AuditEvent' },
      { field: 'resource.kind', header: 'Act On (kind)', class: 'auditevents-column-common auditevents-column-act_on', sortable: false, width: 9, kind: 'AuditEvent' },
      { field: 'resource.name', header: 'Act On (name)', class: 'auditevents-column-common auditevents-column-act_on', sortable: false, width: 9, kind: 'AuditEvent' },
      { field: 'stage', header: 'Stage', class: 'auditevents-column-common auditevents-column-stage', sortable: false, width: 9, kind: 'AuditEvent' },
      { field: 'level', header: 'Level', class: 'auditevents-column-common auditevents-column-level', sortable: false, width: 9 , kind: 'AuditEvent'},
      { field: 'outcome', header: 'Outcome', class: 'auditevents-column-common auditevents-column-outcome', sortable: false, width: 9, kind: 'AuditEvent' },
      { field: 'client-ips', header: 'Client', class: 'auditevents-column-common auditevents-column-client_ips', sortable: false, width: 9 , kind: 'AuditEvent'},
      { field: 'gateway-node', header: 'Service Node', class: 'auditevents-column-common auditevents-column-gateway_node', sortable: false, width: 9 , kind: 'AuditEvent'},
      { field: 'service-name', header: 'Service Name', class: 'auditevents-column-common auditevents-column-service_name', sortable: false, width: 9 , kind: 'AuditEvent', disableSearch: true},
    ];
  }

  function setupNaplesSearch(c: AdvancedSearchComponent) {
    c.cols = [
      { field: 'spec.id', header: 'Name', class: 'naples-column-date', sortable: true, kind: 'DistributedServiceCard' },
      { field: 'status.primary-mac', header: 'MAC Address', class: 'naples-column-id-name', sortable: true, kind: 'DistributedServiceCard' },
      { field: 'status.DSCVersion', header: 'Version', class: 'naples-column-version', sortable: true, kind: 'DistributedServiceCard' },
      { field: 'status.ip-config.ip-address', header: 'Management IP Address', class: 'naples-column-mgmt-cidr', sortable: false, kind: 'DistributedServiceCard' },
      { field: 'status.admission-phase', header: 'Phase', class: 'naples-column-phase', sortable: false, kind: 'DistributedServiceCard' },
      { field: 'status.conditions', header: 'Condition', class: 'naples-column-condition', sortable: true, localSearch: true, kind: 'DistributedServiceCard'},
      { field: 'status.host', header: 'Host', class: 'naples-column-host', sortable: true, kind: 'DistributedServiceCard'},
      { field: 'meta.mod-time', header: 'Modification Time', class: 'naples-column-date', sortable: true, kind: 'DistributedServiceCard' },
      { field: 'meta.creation-time', header: 'Creation Time', class: 'naples-column-date', sortable: true, kind: 'DistributedServiceCard' },
    ];
    c.customQueryOptions = [
      {
        key: {label: 'Condition', value: 'Condition'},
        operators: SearchUtil.stringOperators,
        valueType: ValueType.multiSelect,
        values: [
          { label: 'Healthy', value: NaplesConditionValues.HEALTHY },
          { label: 'Unhealthy', value: NaplesConditionValues.UNHEALTHY },
          { label: 'Unknown', value: NaplesConditionValues.UNKNOWN },
          { label: 'Empty', value: NaplesConditionValues.EMPTY },
        ],
      }
    ];
    c.fieldData = c.generateFieldData(c.customQueryOptions);
    c.genValueLabelToFieldMap();
    c.kind = 'DistributedServiceCard';
  }

  beforeEach(() => {
    fixture = TestBed.createComponent(AdvancedSearchComponent);
    tu = new TestingUtility(fixture);
    component = fixture.componentInstance;
    setupAuditEventInput(component);
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });

  it('should toggle expansion after click', () => {
    let expansion = fixture.debugElement.query(By.css('.advanced-search-expansion'));
    expect(expansion).toBeFalsy();
    const toggleBtn = fixture.debugElement.query(By.css('.advanced-search-bar-button-toggle'));
    tu.sendClick(toggleBtn);
    expansion = fixture.debugElement.query(By.css('.advanced-search-expansion'));
    expect(expansion).toBeTruthy();
  });

  it('should generate correct local search query and results', () => {
    setupNaplesSearch(component);
    const input = fixture.debugElement.query(By.css('.advanced-search-bar-input'));
    tu.setText(input, 'Admitted field:Condition=healthy;');
    const healthy = NaplesConditionValues.HEALTHY;
    const unhealthy = NaplesConditionValues.UNHEALTHY;
    const searchObj = {
      'status.conditions': {
        healthy: ['00ae.cd00.1142'],
        unhealthy: ['00ae.cd00.1146']
      }};
    const remReq = component.getSearchRequest('meta.mod-time', -1, 'DistributedServiceCard');
    const locRes = component.getLocalSearchResult('meta.mod-time', -1, searchObj);
    expect(remReq.query.texts[0].text).toEqual(['"Admitted"']);
    expect(remReq.query.fields.requirements.length).toBe(0);
    expect(locRes.err).toEqual(false);
    expect(locRes.searchRes).toEqual(['00ae.cd00.1142']);
  });

  it('should build field fields without disableSearch', () => {
    // total 11 fields, we are not including disableSearch.
    expect(component.fieldData.length).toBe(10);
  });

  it('should convert search text input and repeater data bidirectionally', () => {
    const input = fixture.debugElement.query(By.css('.advanced-search-bar-input'));
    const toggleBtn = fixture.debugElement.query(By.css('.advanced-search-bar-button-toggle'));

    tu.setText(input, 'field:Client=192.168.74.241;field:Action=Update;');
    tu.sendClick(toggleBtn);
    fixture.detectChanges();

    // first parsed field
    expect(component.formArray.at(0).value['keyFormControl']).toBe('Client');
    expect(component.formArray.at(0).value['operatorFormControl']).toBe('in');
    expect(component.formArray.at(0).value['valueFormControl'][0]).toBe('192.168.74.241');

    // second parsed field
    expect(component.formArray.at(1).value['keyFormControl']).toBe('Action');
    expect(component.formArray.at(1).value['operatorFormControl']).toBe('in');
    expect(component.formArray.at(1).value['valueFormControl'][0]).toBe('Update');

    // delete Action=Update in the expansion
    const secondDeleteBtn = fixture.debugElement.queryAll(By.css('.repeater-trash-icon'))[1].children[0];
    tu.sendClick(secondDeleteBtn);
    // close expansion panel
    tu.sendClick(toggleBtn);
    fixture.detectChanges();
    // search text should updated
    expect(component.search).toBe('field:Client=192.168.74.241;');
  });

  it('should empty the data when user click cancel', () => {
    const input = fixture.debugElement.query(By.css('.advanced-search-bar-input'));
    tu.setText(input, 'field:Client=192.168.74.241;field:Action=Update;');
    fixture.detectChanges();

    // cancel button should show up after user's input
    let cancelBtn = fixture.debugElement.query(By.css('.advanced-search-bar-button-cancel'));
    expect(cancelBtn).toBeTruthy();

    // cancel button should still be visible even when search input is cleared
    tu.sendClick(cancelBtn);
    fixture.detectChanges();
    cancelBtn = fixture.debugElement.query(By.css('.advanced-search-bar-button-cancel'));
    expect(cancelBtn).toBeTruthy();
    expect(component.search).toBeFalsy();
  });

  it('should empty the accept multiple entry in value', () => {
    const input = fixture.debugElement.query(By.css('.advanced-search-bar-input'));
    tu.setText(input, 'field:Client=192.168.74.241;field:Action=Update,test1,test2;');
    fixture.detectChanges();

    expect(component.getValues()[1]['valueFormControl']).toEqual(['Update', 'test1', 'test2']);

    const toggleBtn = fixture.debugElement.query(By.css('.advanced-search-bar-button-toggle'));
    expect(toggleBtn).toBeTruthy();

    tu.sendClick(toggleBtn);
    fixture.detectChanges();
    expect(component.getValues()[1]['valueFormControl']).toEqual(['Update', 'test1', 'test2']);
  });

  it('should parse the complex search input contains keyword and fields to modal', () => {
    const input = fixture.debugElement.query(By.css('.advanced-search-bar-input'));
    tu.setText(input, 'hello field:Who=dsadsa; world field:Act On (kind)=1,2; test');
    const req = component.getSearchRequest('', -1, 'AuditEvent');
    expect(req.query.texts[0].text).toEqual(['"hello"', '"world"', '"test"']);

    const toggleBtn = fixture.debugElement.query(By.css('.advanced-search-bar-button-toggle'));
    tu.sendClick(toggleBtn);
    fixture.detectChanges();

    // first parsed field
    expect(component.formArray.at(0).value['keyFormControl']).toBe('Who');
    expect(component.formArray.at(0).value['operatorFormControl']).toBe('in');
    expect(component.formArray.at(0).value['valueFormControl'][0]).toBe('dsadsa');

    // first parsed field
    expect(component.formArray.at(1).value['keyFormControl']).toBe('Act On (kind)');
    expect(component.formArray.at(1).value['operatorFormControl']).toBe('in');
    expect(component.getValues()[1]['valueFormControl']).toEqual(['1', '2']);
  });
});
