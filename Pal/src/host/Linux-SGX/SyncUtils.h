/*
 * SyncUtils.hpp
 *
 *  Created on: Jun 23, 2016
 *      Author: user
 */

#ifndef ENCLAVE_FORK_TRUSTED_SYNCUTILS_HPP_
#define ENCLAVE_FORK_TRUSTED_SYNCUTILS_HPP_

void rpc_spin_lock(int volatile *p);
void rpc_spin_unlock(int volatile *p);

#endif /* ENCLAVE_FORK_TRUSTED_SYNCUTILS_HPP_ */

