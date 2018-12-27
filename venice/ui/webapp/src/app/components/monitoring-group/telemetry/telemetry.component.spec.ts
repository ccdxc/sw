/**-----
 Angular imports
 ------------------*/
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { MatIconRegistry } from '@angular/material';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { RouterTestingModule } from '@angular/router/testing';
import { SharedModule } from '@app/components/shared/shared.module';
/**-----
 Venice web-app imports
 ------------------*/
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { LogService } from '@app/services/logging/log.service';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { WidgetsModule } from 'web-app-framework';
import { TelemetryComponent } from './telemetry.component';
import { FwlogpoliciesComponent } from './fwlogpolicies/fwlogpolicies.component';
import { NewflowexportComponent } from './flowexport/newflowexport/newflowexport.component';
import { NewfwlogpolicyComponent } from './fwlogpolicies/newfwlogpolicy/newfwlogpolicy.component';
import { FlowexportComponent } from './flowexport/flowexport.component';
import { MessageService } from 'primeng/primeng';
import { MonitoringGroupModule } from '../monitoring-group.module';


describe('TelemetryComponent', () => {
  let component: TelemetryComponent;
  let fixture: ComponentFixture<TelemetryComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [
        TelemetryComponent,
        FwlogpoliciesComponent,
        NewfwlogpolicyComponent,
        FlowexportComponent,
        NewflowexportComponent,
      ],
      imports: [
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
        SharedModule,
        HttpClientTestingModule,
        PrimengModule,
        MaterialdesignModule,
        WidgetsModule,
        RouterTestingModule,
        MonitoringGroupModule
      ],
      providers: [
        ControllerService,
        LogService,
        LogPublishersService,
        MonitoringService,
        MatIconRegistry,
        MessageService
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(TelemetryComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
