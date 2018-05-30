import { HttpClientTestingModule } from '@angular/common/http/testing';
import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { FormsModule } from '@angular/forms';
import { MatIconRegistry } from '@angular/material';
import { RouterTestingModule } from '@angular/router/testing';
import { CoreModule } from '@app/core';

import { AuthService } from '../../services/auth.service';
import { ControllerService } from '../../services/controller.service';
import { LoginComponent } from './login.component';

describe('LoginComponent', () => {
  let component: LoginComponent;
  let fixture: ComponentFixture<LoginComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [LoginComponent],
      imports: [
        RouterTestingModule,
        FormsModule,
        HttpClientTestingModule,
        CoreModule
      ],
      providers: [
        ControllerService,
        AuthService,
        MatIconRegistry
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(LoginComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
