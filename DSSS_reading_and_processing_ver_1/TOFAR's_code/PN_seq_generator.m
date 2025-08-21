%% copyright © ACT Lab, Dept. EE, NTUT 20241105

function PN_code = PN_seq_generator(state_number)
    N = 12;
    % Number of bits to generate
    PN_seq_length = 255;
    if (state_number<17)
        %Initial state (non-zero) last 12 bits left-2-right
        state = [
        1 1 1 0 0 0 1 1 0 0 0 0;
        0 0 1 0 0 1 0 1 0 1 1 1; %(2 optional)
        %0 0 1 1 0 0 0 0 0 1 0 0;
        0 0 0 1 0 1 0 1 0 0 0 0;  
        %0 1 0 0 0 0 1 1 0 0 1 0;
        1 1 0 1 1 0 1 1 1 0 0 1;
        0 0 1 0 1 1 1 0 0 0 1 1;
        0 1 0 1 1 0 1 1 1 1 1 0;
        1 0 0 0 0 0 0 0 1 0 0 1;
        0 0 0 0 1 1 0 0 0 1 1 1;
        1 1 0 0 1 1 0 0 0 1 1 1;
        1 1 0 1 0 0 0 0 1 0 0 0;
        1 1 0 1 0 0 0 1 0 0 1 1;
        1 1 0 1 0 0 1 1 0 0 0 0;
        1 1 1 0 1 0 1 0 1 1 0 1; 
        1 1 1 0 1 1 1 0 0 1 0 1;
        1 1 1 1 0 0 0 0 1 1 1 1; 
        1 1 1 1 0 0 1 1 1 1 0 1;
        ]';
        
        % state_0 = [1 1 1 0 0 0 1 1 0 0 0 0]'; %12M5 original
        % state_0F = flip(state_0); 
        % state_0R = ~state_0;
        % state_0RF = flip(state_0R);
        
        % Define feedback taps according to the polynomial 
        feedback = [0 0 1 0 1 1 1 1 0 1 1 1]'; % Taps corresponding to the polynomial 
        
        % LFSR implementation
        pnSequence = NaN(PN_seq_length,16);
        for pnidx = 1:16
            for i = 1:PN_seq_length
                % Output is the current state (rightmost bit)
                pnSequence(i,pnidx) = state(end,pnidx);
                
                % Calculate new input to the register (XOR of taps)
                newBit = mod(sum(and(state(:,pnidx), feedback)),2); % XOR of tap bits
                
                % Shift the register to the right
                state(:,pnidx) = [newBit; state(1:end-1,pnidx)];
                
            end
        end

        % %% --- xcorr validation----
        % correlation_matrix = NaN(16,16);
        % for pnidx = 1:16
        %     pnSequence_A = pnSequence(:,pnidx);
        %     for pnidx_2 = 1:16
        %         pnSequence_B = pnSequence(:,pnidx_2);
        %         correlation_matrix(pnidx,pnidx_2) =  max(abs(cyclic_xcorr(pnSequence_A,pnSequence_B)-127.5)+127.5)>=PN_seq_length;
        %     end
        % end
        % %%
        PN_code = flip(pnSequence(:,state_number));
    elseif(state_number==17)
        PN_code = [0 1 0 1 0 1 0 0 0 1 0 0 1 0 0 1 0 0 1 0 1 1 1 0 0 0 0 0 1 0 0 0 1 0 0 0 0 1 1 0 1 0 0 1 0 0 1 1 0 1 1 1 1 0 1 1 0 1 1 0 1 1 1 1 0 1 1 1 0 1 1 1 0 0 0 0 1 1 0 0 1 1 0 1 0 1 1 0 1 0 0 1 1 0 0 1 0 0 0 1 1 1 0 0 1 1 0 1 1 0 1 0 0 1 0 0 0 1 1 0 0 0 1 0 0 1 0 0 0 0 1 0 1 0 1 1 1 0 0 1 1 1 0 1 0 0 1 0 0 1 1 1 0 1 0 1 1 0 0 0 1 1 1 1 1 1 0 1 1 0 0 1 1 1 1 0 0 0 1 0 0 1 0 1 0 1 0 1 0 0 1 0 1 0 1 0 1 1 1 1 1 1 1 0 1 1 1 1 0 1 1 1 1 0 1 0 0 1 1 1 1 1 0 1 1 1 0 0 1 1 0 1 0 0 0 1 1 0 0 0 1 1 1 1 1 0 1 1 0 0 1 0 0 1 1]';
    end

end
