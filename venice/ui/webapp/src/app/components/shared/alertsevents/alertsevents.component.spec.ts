/**-----
 Angular imports
 ------------------*/
import { ComponentFixture, TestBed, async, fakeAsync } from '@angular/core/testing';
import { AlertseventsComponent } from './alertsevents.component';
import { Component } from '@angular/core';
import { configureTestSuite } from 'ng-bullet';
import { SharedModule } from '@app/components/shared/shared.module';
import { ControllerService } from '@app/services/controller.service';
import { RouterTestingModule } from '@angular/router/testing';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MessageService } from '@app/services/message.service';
import { ConfirmationService } from 'primeng/primeng';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { AuthService } from '@app/services/auth.service';
import { HttpClientModule } from '@angular/common/http';

@Component({
  template: ''
})
class DummyComponent { }

describe('AlertsEventsComponent', () => {
  let component: AlertseventsComponent;
  let fixture: ComponentFixture<AlertseventsComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [
        DummyComponent,
      ],
      imports: [
        HttpClientModule,
        SharedModule,

        RouterTestingModule.withRoutes([
          { path: 'login', component: DummyComponent }
        ]),
      ],
      providers: [
        ControllerService,
        LogService,
        LogPublishersService,
        MessageService,
        ConfirmationService,
        UIConfigsService,
        LicenseService,
        AuthService
      ]
    });
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(AlertseventsComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });

});
