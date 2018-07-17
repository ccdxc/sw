/**-----
 Angular imports
 ------------------*/
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { MatIconRegistry } from '@angular/material';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { RouterTestingModule } from '@angular/router/testing';
import { DestinationpolicyComponent } from '@app/components/monitoring-group/alertsevents/alertpolicies/destinations/destinations.component';
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
import { EventalertpolicyComponent } from '../alertpolicies/eventalertpolicies/eventalertpolicies.component';
import { AlertpoliciesComponent } from './alertpolicies.component';
import { MetricpolicyComponent } from './metricpolicy/metricpolicy.component';
import { ObjectpolicyComponent } from './objectpolicy/objectpolicy.component';
import { NeweventalertpolicyComponent } from '@app/components/monitoring-group/alertsevents/alertpolicies/eventalertpolicies/neweventalertpolicy/neweventalertpolicy.component';


describe('AlertpoliciesComponent', () => {
  let component: AlertpoliciesComponent;
  let fixture: ComponentFixture<AlertpoliciesComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [AlertpoliciesComponent,
        EventalertpolicyComponent,
        MetricpolicyComponent,
        ObjectpolicyComponent,
        DestinationpolicyComponent,
        NeweventalertpolicyComponent
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
        RouterTestingModule
      ],
      providers: [
        ControllerService,
        LogService,
        LogPublishersService,
        MonitoringService,
        MatIconRegistry,
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(AlertpoliciesComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
