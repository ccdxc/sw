/**-----
 Angular imports
 ------------------*/
import { async, ComponentFixture, TestBed, inject } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { FormsModule } from '@angular/forms';
import { Observable } from 'rxjs/Observable';
import { BrowserAnimationsModule } from '@angular/platform-browser/animations';
/**-----
 VeniceUI Framework -  imports
 ------------------*/
import { WidgetsModule } from 'web-app-framework';

/**-----
 Venice UI -  imports
 ------------------*/
import { ControllerService } from '@app/services/controller.service';
import { WorkloadService } from '@app/services/generated/workload.service';

import { WorkloadComponent } from './workload.component';
import { WorkloadwidgetComponent } from './workloadwidget/workloadwidget.component';
import { SharedModule } from '@app/components/shared//shared.module';
import { MockDataUtil } from '@app/common/MockDataUtil';

/**-----
 Third-parties imports
 ------------------*/
import { MomentModule } from 'angular2-moment';
import { MatIconRegistry } from '@angular/material';


import { PrimengModule } from '@lib/primeng.module';
import { Component } from '@angular/core';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
@Component({
  template: ''
})
class DummyComponent { }

describe('WorkloadComponent', () => {
  let component: WorkloadComponent;
  let fixture: ComponentFixture<WorkloadComponent>;

  let testBedService: WorkloadService;
  let componentService: WorkloadService;


  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [WorkloadComponent,
        WorkloadwidgetComponent,
        DummyComponent
      ],
      imports: [
        RouterTestingModule.withRoutes([
          { path: 'login', component: DummyComponent }
        ]),
        FormsModule,
        HttpClientTestingModule,
        // VeniceUI Framework
        WidgetsModule,
        // Moment.js
        MomentModule,
        // primeNG.js
        PrimengModule,
        MaterialdesignModule,
        SharedModule,
        BrowserAnimationsModule
      ],
      providers: [
        ControllerService,
        LogService,
        LogPublishersService,
        WorkloadService,
        MatIconRegistry,
        UIConfigsService
      ]
    })
      .compileComponents();

  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(WorkloadComponent);
    component = fixture.componentInstance;
  });

  afterEach(() => {
    testBedService = null;
    componentService = null;
    component = null;
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });

});
