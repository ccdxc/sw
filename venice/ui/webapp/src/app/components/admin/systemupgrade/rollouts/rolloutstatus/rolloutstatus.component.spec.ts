import { ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { SharedModule } from '@app/components/shared/shared.module';
import { ReactiveFormsModule, FormsModule } from '@angular/forms';
import { ControllerService } from '@app/services/controller.service';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { ClusterDistributedServiceCard } from '@sdk/v1/models/generated/cluster';
import { RolloutService } from '@app/services/generated/rollout.service';
import { MatIconRegistry } from '@angular/material';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { ConfirmationService } from 'primeng/primeng';
import { PrimengModule } from '@app/lib/primeng.module';
import { MessageService } from '@app/services/message.service';
import { ObjstoreService } from '@app/services/generated/objstore.service';
import { AuthService } from '@app/services/auth.service';

import { RolloutstatusComponent } from './rolloutstatus.component';

describe('RolloutstatusComponent', () => {
  let component: RolloutstatusComponent;
  let fixture: ComponentFixture<RolloutstatusComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [RolloutstatusComponent],
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
        UIConfigsService,
        LicenseService,
        RolloutService,
        AuthService,
        ClusterService
      ]
    });
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(RolloutstatusComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
