import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { SharedModule } from '@app/components/shared/shared.module';
import { ReactiveFormsModule, FormsModule } from '@angular/forms';
import { ControllerService } from '@app/services/controller.service';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MatIconRegistry } from '@angular/material';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { ConfirmationService } from 'primeng/primeng';
import { PrimengModule } from '@app/lib/primeng.module';
import { MessageService } from '@app/services/message.service';

import { SystemupgradeComponent } from './systemupgrade.component';

describe('SystemupgradeComponent', () => {
  let component: SystemupgradeComponent;
  let fixture: ComponentFixture<SystemupgradeComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [ SystemupgradeComponent ],
      imports: [
        MaterialdesignModule,
        RouterTestingModule,
        HttpClientTestingModule,
        SharedModule,
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
        PrimengModule
      ],
      providers: [
        ControllerService,
        LogService,
        LogPublishersService,
        MessageService,
        ConfirmationService,
        MatIconRegistry,
      ]
    });
    });

  beforeEach(() => {
    fixture = TestBed.createComponent(SystemupgradeComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
