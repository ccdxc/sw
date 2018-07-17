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
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { LogService } from '@app/services/logging/log.service';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { EventalertpolicyComponent } from './eventalertpolicies.component';
import { NeweventalertpolicyComponent } from '@app/components/monitoring-group/alertsevents/alertpolicies/eventalertpolicies/neweventalertpolicy/neweventalertpolicy.component';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { WidgetsModule } from 'web-app-framework';


describe('EventalertpolicyComponent', () => {
  let component: EventalertpolicyComponent;
  let fixture: ComponentFixture<EventalertpolicyComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [EventalertpolicyComponent, NeweventalertpolicyComponent],
      imports: [
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
        SharedModule,
        HttpClientTestingModule,
        PrimengModule,
        MaterialdesignModule,
        RouterTestingModule,
        WidgetsModule
      ],
      providers: [
        ControllerService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        MonitoringService
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(EventalertpolicyComponent);
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
});
