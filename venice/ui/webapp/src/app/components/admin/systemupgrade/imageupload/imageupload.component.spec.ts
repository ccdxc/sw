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
import { ObjstoreService } from '@app/services/generated/objstore.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { AuthService as AuthServiceGen } from '@app/services/generated/auth.service';
import { AuthService } from '@app/services/auth.service';
import { ImageuploadComponent } from './imageupload.component';
import { RolloutService } from '@app/services/generated/rollout.service';

describe('ImageuploadComponent', () => {
  let component: ImageuploadComponent;
  let fixture: ComponentFixture<ImageuploadComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [ ImageuploadComponent ],
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
        ObjstoreService,
        AuthService,
        AuthServiceGen,
        UIConfigsService,
        LicenseService,
        RolloutService
      ]
    });
    });

  beforeEach(() => {
    fixture = TestBed.createComponent(ImageuploadComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
