import { HttpClientTestingModule } from '@angular/common/http/testing';
import { Component } from '@angular/core';
/**-----
 Angular imports
 ------------------*/
import { ComponentFixture, TestBed, async } from '@angular/core/testing';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { MatIconRegistry } from '@angular/material';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { RouterTestingModule } from '@angular/router/testing';
import { SharedModule } from '@app/components/shared/shared.module';
import { Rule, RuleAction } from '@app/models/frontend/shared/rule.interface';
/**-----
 Venice web-app imports
 ------------------*/
import { ControllerService } from '@app/services/controller.service';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { SecurityService } from 'app/services/security.service';
import { SgpolicyComponent } from './sgpolicy.component';





@Component({
  template: ''
})
class DummyComponent { }

describe('SgpolicyComponent', () => {
  let component: SgpolicyComponent;
  let fixture: ComponentFixture<SgpolicyComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [SgpolicyComponent, DummyComponent],
      imports: [
        RouterTestingModule.withRoutes([
          { path: 'login', component: DummyComponent }
        ]),
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
        SecurityService,
        MatIconRegistry,
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(SgpolicyComponent);
    component = fixture.componentInstance;
    component.cols = [
      { field: 'meta', header: 'Meta' },
      { field: 'spec', header: 'Spec' },
      { field: 'status', header: 'Status' }
    ];
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });

  // Testing IP Search
  it('IP search testing', () => {
    // blank search
    let ip = '';
    let protocolPort = '';
    let search = [ip, protocolPort];
    const listReference: Rule = {
      sourceIP: {
        list: ['192.168.10.2']
      },
      destIP: {
        list: ['10.10.10.10']
      },
      ports: [{
        protocol: 'TCP',
        list: [8000]
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
        }]
      }],
      action: RuleAction.permit
    };
    expect(component.ipSearch(listReference, search)).toBeTruthy();

    // exact source match search, no port
    ip = '192.168.10.2';
    protocolPort = '';
    search = [ip, protocolPort];
    expect(component.ipSearch(listReference, search)).toBeTruthy();

    // false search, no port
    ip = '10.10.10.2';
    protocolPort = '';
    search = [ip, protocolPort];
    expect(component.ipSearch(listReference, search)).toBeFalsy();

    // prefix search, no port
    ip = '192.16';
    protocolPort = '';
    search = [ip, protocolPort];
    expect(component.ipSearch(listReference, search)).toBeTruthy();

    // mask search IP, no port
    ip = '192.168.10.200';
    protocolPort = '';
    search = [ip, protocolPort];
    expect(component.ipSearch(maskReference, search)).toBeTruthy();

    // protocol search, no ip
    ip = '';
    protocolPort = 'TCP/8000';
    search = [ip, protocolPort];
    expect(component.ipSearch(listReference, search)).toBeTruthy();

    ip = '';
    protocolPort = '8000';
    search = [ip, protocolPort];
    expect(component.ipSearch(listReference, search)).toBeTruthy();

    ip = '';
    protocolPort = 'TCP';
    search = [ip, protocolPort];
    expect(component.ipSearch(listReference, search)).toBeTruthy();

    // partial match, no ip
    ip = '';
    protocolPort = 'TC 80';
    search = [ip, protocolPort];
    expect(component.ipSearch(listReference, search)).toBeTruthy();

    // range search, no ip
    ip = '';
    protocolPort = 'TCP/8500';
    search = [ip, protocolPort];
    expect(component.ipSearch(rangeReference, search)).toBeTruthy();

    // wrong protocol, no ip
    ip = '';
    protocolPort = 'UDP 8000';
    search = [ip, protocolPort];
    expect(component.ipSearch(listReference, search)).toBeFalsy();

    // wrong port, no ip
    ip = '';
    protocolPort = 'TCP 6000';
    search = [ip, protocolPort];
    expect(component.ipSearch(listReference, search)).toBeFalsy();

    // Right IP wrong port
    ip = '192.168.10.2';
    protocolPort = 'TCP 6000';
    search = [ip, protocolPort];
    expect(component.ipSearch(listReference, search)).toBeFalsy();

    // wrong IP right port
    ip = '192.188.10.2';
    protocolPort = 'TCP 8000';
    search = [ip, protocolPort];
    expect(component.ipSearch(listReference, search)).toBeFalsy();

    // Right IP right port
    ip = '192.168.10.2';
    protocolPort = 'TCP 8000';
    search = [ip, protocolPort];
    expect(component.ipSearch(listReference, search)).toBeTruthy();
  });

});
