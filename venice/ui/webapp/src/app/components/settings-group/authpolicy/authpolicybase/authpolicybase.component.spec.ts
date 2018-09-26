import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { AuthpolicybaseComponent } from './authpolicybase.component';

describe('AuthpolicybaseComponent', () => {
  let component: AuthpolicybaseComponent;
  let fixture: ComponentFixture<AuthpolicybaseComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [AuthpolicybaseComponent]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(AuthpolicybaseComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should lower rank', () => {
    const spy = spyOn(component.changeAuthRank, 'emit');
    component.currentRank = 0;
    component.numRanks = 2;
    expect(component.canLowerAuthRank()).toBeTruthy();
    expect(component.canRaiseAuthRank()).toBeFalsy();
    component.raiseAuthRank();
    expect(spy).toHaveBeenCalledTimes(0);
    component.lowerAuthRank();
    expect(spy).toHaveBeenCalledTimes(1);
    expect(spy).toHaveBeenCalledWith(1);
  });

  it('should raise rank', () => {
    const spy = spyOn(component.changeAuthRank, 'emit');
    component.currentRank = 1;
    component.numRanks = 2;
    expect(component.canLowerAuthRank()).toBeFalsy();
    expect(component.canRaiseAuthRank()).toBeTruthy();
    component.lowerAuthRank();
    expect(spy).toHaveBeenCalledTimes(0);
    component.raiseAuthRank();
    expect(spy).toHaveBeenCalledTimes(1);
    expect(spy).toHaveBeenCalledWith(0);
  });

  it('should handle missing input gracefully', () => {
    const spy = spyOn(component.changeAuthRank, 'emit');
    component.numRanks = 2;
    expect(component.canLowerAuthRank()).toBeFalsy();
    expect(component.canRaiseAuthRank()).toBeFalsy();
    component.raiseAuthRank();
    expect(spy).toHaveBeenCalledTimes(0);
    component.lowerAuthRank();
    expect(spy).toHaveBeenCalledTimes(0);

    component.currentRank = 1;
    component.numRanks = null;
    expect(component.canLowerAuthRank()).toBeFalsy();
    expect(component.canRaiseAuthRank()).toBeFalsy();
    component.raiseAuthRank();
    expect(spy).toHaveBeenCalledTimes(0);
    component.lowerAuthRank();
    expect(spy).toHaveBeenCalledTimes(0);
  });
});
