import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { Component } from '@angular/core';

import { HttpClientTestingModule } from '@angular/common/http/testing';

import { GridsterModule } from 'angular-gridster2';
import { WidgetsModule } from 'web-app-framework';
import { PrimengModule } from '@lib/primeng.module';
import { MaterialdesignModule } from '../../lib/materialdesign.module';

import { FormsModule } from '@angular/forms';
import { ControllerService } from '../../services/controller.service';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { SharedModule } from '@components/shared/shared.module';

import { DashboardComponent } from './dashboard.component';
import { SoftwareversionComponent } from './widgets/softwareversion/softwareversion.component';
import { SystemcapacitywidgetComponent } from './widgets/systemcapacity/systemcapacity.component';
import { NaplesComponent } from './widgets/naples/naples.component';
import { PolicyhealthComponent } from './widgets/policyhealth/policyhealth.component';
import { DsbdworkloadComponent } from './widgets/dsbdworkload/dsbdworkload.component';

import { MatIconRegistry } from '@angular/material';

@Component({
  template: ''
})
class DummyComponent { }

describe('DashboardComponent', () => {
  let component: DashboardComponent;
  let fixture: ComponentFixture<DashboardComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [DashboardComponent,
        SystemcapacitywidgetComponent,
        SoftwareversionComponent,
        NaplesComponent,
        PolicyhealthComponent,
        DsbdworkloadComponent,
        DummyComponent],
      imports: [

        RouterTestingModule.withRoutes([
          { path: 'login', component: DummyComponent }
        ]),
        FormsModule,
        HttpClientTestingModule,
        GridsterModule,
        WidgetsModule,
        PrimengModule,
        MaterialdesignModule,
        SharedModule
      ],
      providers: [
        ControllerService,
        MatIconRegistry,
        LogService,
        LogPublishersService
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(DashboardComponent);
    component = fixture.componentInstance;
    component.gridsterOptions = [];
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
