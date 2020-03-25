import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { PrimengModule } from '@app/lib/primeng.module';
import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { NewdscprofileComponent } from './newdscprofile.component';
import { SharedModule } from '@app/components/shared/shared.module';
import { ReactiveFormsModule, FormsModule } from '@angular/forms';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { ControllerService } from '@app/services/controller.service';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MessageService } from '@app/services/message.service';
import { MatIconRegistry } from '@angular/material';
import { ConfirmationService } from 'primeng/api';
import { AuthService } from '@app/services/auth.service';

describe('NewdscprofileComponent', () => {
  let component: NewdscprofileComponent;
  let fixture: ComponentFixture<NewdscprofileComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [NewdscprofileComponent],
      imports: [
        SharedModule,
        ReactiveFormsModule,
        RouterTestingModule,
        HttpClientTestingModule,
        FormsModule,
        NoopAnimationsModule,
        PrimengModule,
        MaterialdesignModule
      ],
      providers: [
        ControllerService,
        ClusterService,
        UIConfigsService,
        LogService,
        ControllerService,
        LogPublishersService,
        MatIconRegistry,
        ConfirmationService,
        MessageService,
        AuthService,
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(NewdscprofileComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create NewdscprofileComponent', () => {
    expect(component).toBeTruthy();
  });
});
