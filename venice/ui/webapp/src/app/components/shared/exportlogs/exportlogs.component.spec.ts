import { ComponentFixture, TestBed } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { PrimengModule } from '@app/lib/primeng.module';
import { configureTestSuite } from 'ng-bullet';
import { ExportLogsComponent } from './exportlogs.component';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { NO_ERRORS_SCHEMA } from '@angular/core';
import { ControllerService } from '@app/services/controller.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { SecurityService } from '@app/services/generated/security.service';
import { LogService } from '@app/services/logging/log.service';
import { MessageService } from '@app/services/message.service';
import { ConfirmationService } from 'primeng/primeng';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { AuthService } from '@app/services/auth.service';
import { HttpClient, HttpHandler } from '@angular/common/http';


describe('ExportLogsComponent', () => {
  let component: ExportLogsComponent;
  let fixture: ComponentFixture<ExportLogsComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [ExportLogsComponent],
      imports: [
        RouterTestingModule,
        PrimengModule,
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
      ],
      schemas: [NO_ERRORS_SCHEMA],
      providers: [
        ControllerService,
        UIConfigsService,
        MonitoringService,
        SecurityService,
        LogService,
        LogPublishersService,
        MessageService,
        ConfirmationService,
        AuthService,
        HttpClient,
        HttpHandler
      ]
    });
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(ExportLogsComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
